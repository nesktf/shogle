#include "./internal/common.hpp"
#include "./internal/opengl/context.hpp"

namespace ntf {

static constexpr size_t INITIAL_ARENA_PAGE = mibs(4u);

r_allocator rp_alloc::base_alloc {
  .user_ptr = nullptr,
  .mem_alloc = +[](void*, size_t sz, size_t) -> void* { return std::malloc(sz); },
  .mem_free = +[](void*, void* mem) -> void { std::free(mem); }
};

static r_expected<r_platform_context*> load_platform_ctx(
  rp_alloc& alloc, r_api api, r_window win, uint32 swap_interval
) {
  RET_ERROR_IF(!win, "Invalid window handle");
  r_platform_context* ctx;
  try {
    switch (api) {
      case r_api::opengl: {
        SHOGLE_GL_MAKE_CTX_CURRENT(win);
        SHOGLE_GL_SET_SWAP_INTERVAL(win, static_cast<int>(swap_interval));
        RET_ERROR_IF(!gladLoadGLLoader(SHOGLE_GL_LOAD_PROC),
                     "Failed to load GLAD");
        ctx = alloc.construct<gl_context>(win, swap_interval);
        break;
      }
      default: {
        RET_ERROR("Renderer not implemented");
        break;
      }
    }
    NTF_ASSERT(ctx);
  }
  RET_ERROR_CATCH("Failed to load platform context");

  // TODO: Let the user handle the imgui context
  //       (and move imgui platform calls to their respective place)
#if defined(SHOGLE_ENABLE_IMGUI) && SHOGLE_ENABLE_IMGUI
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();

  const bool bind_callbacks = true;

  switch (api) {
    case r_api::opengl: {
      SHOGLE_INIT_IMGUI_OPENGL(win, bind_callbacks);
      break;
    }
    case r_api::vulkan: {
      SHOGLE_INIT_IMGUI_VULKAN(win, bind_callbacks);
      break;
    }
    default: {
      SHOGLE_INIT_IMGUI_OTHER(win, bind_callbacks);
      break;
    }
  }
#endif

  return ctx;
}

r_expected<r_context> r_create_context(const r_context_params& params) {
  auto alloc = params.alloc ? rp_alloc{*params.alloc} : rp_alloc{};
  RET_ERROR_IF(!alloc.arena_init(INITIAL_ARENA_PAGE), "Failed to allocate scratch arena");

  return load_platform_ctx(alloc, params.renderer_api, params.window, params.swap_interval)
  .and_then([&](auto&& renderer) -> r_expected<r_context> {
    r_context ctx = nullptr;
    {
      ctx = alloc.allocate_uninited<r_context_>();
      RET_ERROR_IF(!ctx, "Failed to allocate context");

      rp_command_map map; // TODO: use the allocator for this thing
      auto [it, emplaced] = map.try_emplace(DEFAULT_FBO_HANDLE);
      NTF_ASSERT(emplaced);
      it->second.color = params.fb_color;
      it->second.viewport = params.fb_viewport;
      it->second.clear = params.fb_clear;

      std::construct_at(ctx,
                        *renderer, std::move(map),
                        params.window, params.renderer_api,
                        std::move(alloc));
    }

    rp_platform_meta meta;
    ctx->renderer().get_meta(meta);
    SHOGLE_LOG(debug, "[ntf::r_context][CONSTRUCT] {} ver {} [{} - {}]",
               meta.api == r_api::opengl ? "OpenGL" : "Vulkan",
               meta.version_str, meta.vendor_str, meta.name_str);
    return ctx;
  });
}

void r_destroy_context(r_context ctx) {
  if (!ctx) {
    return;
  }

#if defined(SHOGLE_ENABLE_IMGUI) && SHOGLE_ENABLE_IMGUI
  switch (ctx->api()) {
    case r_api::opengl: {
      SHOGLE_DESTROY_IMGUI_OPENGL();
      break;
    }
    case r_api::vulkan: {
      SHOGLE_DESTROY_IMGUI_VULKAN();
      break;
    }
    default: {
      SHOGLE_DESTROY_IMGUI_OTHER();
      break;
    }
  }
  ImGui::DestroyContext();
#endif

  auto alloc = std::move(ctx->alloc());
  auto& renderer = ctx->renderer();
  alloc.destroy(ctx);
  alloc.destroy(&renderer);
  alloc.arena_destroy();
  SHOGLE_LOG(debug, "[ntf::r_context][DESTROY]");
}

void r_start_frame(r_context ctx) {
  if (!ctx) {
    return;
  }

  for (auto& [_, list] : ctx->draw_lists()) {
    list.cmds.clear();
  }
  ctx->alloc().arena_clear();

#if defined(SHOGLE_ENABLE_IMGUI) && SHOGLE_ENABLE_IMGUI
  switch (ctx->api()) {
    case r_api::opengl: {
      SHOGLE_IMGUI_OPENGL_NEW_FRAME();
      break;
    }
    case r_api::vulkan: {
      SHOGLE_IMGUI_VULKAN_NEW_FRAME();
      break;
    }
    default: {
      SHOGLE_IMGUI_OTHER_NEW_FRAME();
      break;
    }
  }
  ImGui::NewFrame();
#endif
}

void r_end_frame(r_context ctx) {
  if (!ctx) {
    return;
  }

  // TODO: Sort the draw lists before submiting
  ctx->renderer().submit(ctx->draw_lists());

#if defined(SHOGLE_ENABLE_IMGUI) && SHOGLE_ENABLE_IMGUI
  ImGui::Render();
  switch (ctx->api()) {
    case r_api::opengl: {
      SHOGLE_IMGUI_OPENGL_END_FRAME(ImGui::GetDrawData());
      break;
    }
    case r_api::vulkan: {
      SHOGLE_IMGUI_VULKAN_END_FRAME(ImGui::GetDrawData());
      break;
    }
    default: {
      SHOGLE_IMGUI_OTHER_END_FRAME(ImGui::GetDrawData());
    }
  }
#endif

  ctx->renderer().swap_buffers();
}

void r_device_wait(r_context ctx) {
  if (!ctx) {
    return;
  }
  ctx->renderer().device_wait();
}

void r_submit_command(r_context ctx, const r_draw_command& cmd) {
  if (!ctx) {
    return;
  }

  auto& alloc = ctx->alloc();
  auto it = ctx->draw_lists().find(cmd.target->handle);
  NTF_ASSERT(it != ctx->draw_lists().end());

  it->second.cmds.emplace_back(); // TODO: Use the allocator here too
  auto& d_cmd = it->second.cmds.back();

  d_cmd.ctx = ctx;
  d_cmd.pipeline = cmd.pipeline->handle;
  d_cmd.count = cmd.draw_opts.count;
  d_cmd.offset = cmd.draw_opts.offset;
  d_cmd.instances = cmd.draw_opts.instances;
  d_cmd.sort_group = cmd.draw_opts.sort_group;
  d_cmd.on_render = cmd.on_render;

  if (!cmd.buffers.empty()) {
    const size_t buff_count = cmd.buffers.size();
    auto* buffers = alloc.arena_allocate_uninited<rp_buffer_binding>(buff_count);
    for (size_t i = 0u; const auto& buff : cmd.buffers) {
      std::construct_at(buffers+i,
                        buff.buffer->handle, buff.type, buff.location);
      ++i;
    }
    d_cmd.buffers = {buffers, buff_count};
  }

  if (!cmd.textures.empty()) {
    const size_t tex_count = cmd.textures.size();
    auto* textures = alloc.arena_allocate_uninited<rp_texture_binding>(tex_count);
    for (size_t i = 0u; const auto& tex : cmd.textures) {
      std::construct_at(textures+i,
                        tex.texture->handle, tex.location);
      ++i;
    }
    d_cmd.textures = {textures, tex_count};
  }

  if (!cmd.uniforms.empty()) {
    const size_t uniform_count = cmd.uniforms.size();
    auto* unifs = alloc.arena_allocate_uninited<rp_uniform_binding>(uniform_count);
    for (size_t i = 0u; const auto& unif : cmd.uniforms) {
      const size_t data_sz = unif.data.size;
      const size_t data_align = unif.data.alignment;
      auto* data = alloc.arena_allocate(data_sz, data_align);
      std::memcpy(data, unif.data.data, data_sz);
      std::construct_at(unifs+i,
                        unif.uniform->location, unif.uniform->type,
                        data, data_sz);
    }
    d_cmd.uniforms = {unifs, uniform_count};
  }
}

} // namespace
