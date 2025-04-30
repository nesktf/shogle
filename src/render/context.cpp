#include "./common.hpp"
#include "./opengl/context.hpp"

namespace ntf {

static auto load_platform_ctx(
  renderer_api api, win_handle win, uint32 swap_interval
) -> r_expected<std::unique_ptr<r_platform_context>> {
  std::unique_ptr<r_platform_context> ctx;
  try {
    switch (api) {
      case renderer_api::opengl: {
        SHOGLE_GL_MAKE_CTX_CURRENT(win);
        SHOGLE_GL_SET_SWAP_INTERVAL(win, static_cast<int>(swap_interval));
        RET_ERROR_IF(!gladLoadGLLoader(SHOGLE_GL_LOAD_PROC),
                     "[ntf::load_platform_ctx]",
                     "Failed to load GLAD");
        ctx = std::make_unique<gl_context>(win, swap_interval);
        break;
      }
      default: {
        return unexpected{r_error{"Not implemented"}};
        break;
      }
    }
  }
  RET_ERROR_CATCH("[ntf::load_platform_ctx]",
                  "Failed to load platform context");

  // TODO: Use a local imgui context instead of the global one
#if defined(SHOGLE_ENABLE_IMGUI) && SHOGLE_ENABLE_IMGUI
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();

  switch (api) {
    case renderer_api::opengl: {
      SHOGLE_INIT_IMGUI_OPENGL(win, true, "#version 130");
      break;
    }
    case renderer_api::vulkan: {
      SHOGLE_INIT_IMGUI_VULKAN(win, true);
      break;
    }
    default: {
      SHOGLE_INIT_IMGUI_OTHER(win, true);
      break;
    }
  }
#endif

  return ctx;
}

static r_allocator base_alloc {
  .user_ptr = nullptr,
  .mem_alloc = +[](void*, size_t sz, size_t) -> void* { return std::malloc(sz); },
  .mem_free = +[](void*, void* mem) -> void { std::free(mem); },
  .mem_scratch_alloc = +[](void*, size_t sz, size_t) -> void* { return std::malloc(sz); },
  .mem_scratch_free = +[](void*, void* mem) -> void { std::free(mem); },
};

r_expected<r_context> r_create_context(const r_context_params& params) {
  return load_platform_ctx(params.api, params.window, params.swap_interval)
    .and_then([&](auto&& pctx) -> r_expected<r_context> {
      r_allocator alloc = params.alloc ? *params.alloc : base_alloc;
      r_context ctx = static_cast<r_context>(
        (*alloc.mem_alloc)(alloc.user_ptr, sizeof(r_context_), alignof(r_context_))
      );
      if (!ctx) {
        return unexpected{r_error{"Failed to allocate context"}};
      }

      command_map map; // TODO: use the alloator for this thing
      auto [it, emplaced] = map.try_emplace(DEFAULT_FBO_HANDLE);
      NTF_ASSERT(emplaced);

      auto arena = linked_arena::from_size(mibs(4u));
      if (!arena) {
        return unexpected{std::move(arena.error())};
      }
      std::construct_at(ctx,
                        std::move(pctx), std::move(map), params.window, params.api,
                        std::move(*arena),
                        alloc);
      ctx->d_list = ctx->draw_lists.at(DEFAULT_FBO_HANDLE);
      ctx->d_list->color = params.fb_color;
      ctx->d_list->viewport = params.fb_viewport;
      ctx->d_list->clear = params.fb_clear;
      auto meta = ctx->platform->get_meta();
      SHOGLE_LOG(debug, "[ntf::r_context][CONSTRUCT] {} ver {} [{} - {}]",
                 meta.api == renderer_api::opengl ? "OpenGL" : "Vulkan",
                 meta.version_str, meta.vendor_str, meta.name_str);
      return ctx;
    });
}

void r_destroy_context(r_context ctx) {
  if (!ctx) {
    return;
  }
  r_allocator alloc = ctx->alloc;

#if defined(SHOGLE_ENABLE_IMGUI) && SHOGLE_ENABLE_IMGUI
  switch (ctx->api) {
    case renderer_api::opengl: {
      SHOGLE_DESTROY_IMGUI_OPENGL();
      break;
    }
    case renderer_api::vulkan: {
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
  ctx->~r_context_();
  (*alloc.mem_free)(alloc.user_ptr, static_cast<void*>(ctx));
  SHOGLE_LOG(debug, "[ntf::r_context][DESTROY]");
}

void r_start_frame(r_context ctx) {
  if (!ctx) {
    return;
  }

  for (auto& [_, list] : ctx->draw_lists) {
    for (auto& cmd : list.cmds) {
      cmd.get().~draw_command();
    }
    list.cmds.clear();
  }
  ctx->frame_arena.clear();

#if defined(SHOGLE_ENABLE_IMGUI) && SHOGLE_ENABLE_IMGUI
  switch (ctx->api) {
    case renderer_api::opengl: {
      SHOGLE_IMGUI_OPENGL_NEW_FRAME();
      break;
    }
    case renderer_api::vulkan: {
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
  ctx->platform->submit(ctx->draw_lists);
#if defined(SHOGLE_ENABLE_IMGUI) && SHOGLE_ENABLE_IMGUI
  ImGui::Render();
  switch (ctx->api) {
    case renderer_api::opengl: {
      SHOGLE_IMGUI_OPENGL_END_FRAME(ImGui::GetDrawData());
      break;
    }
    case renderer_api::vulkan: {
      SHOGLE_IMGUI_VULKAN_END_FRAME(ImGui::GetDrawData());
      break;
    }
    default: {
      SHOGLE_IMGUI_OTHER_END_FRAME(ImGui::GetDrawData());
    }
  }
#endif
  ctx->platform->swap_buffers();
}

void r_device_wait(r_context ctx) {
  if (!ctx) {
    return;
  }
  ctx->platform->device_wait();
}

void r_submit_command(r_context ctx, const r_draw_command& cmd) {
  if (!ctx) {
    return;
  }

  ctx->d_list = ctx->draw_lists.at(cmd.target->handle);
  ctx->d_cmd.pipeline = cmd.pipeline;

  ctx->d_cmd.count = cmd.draw_opts.count;
  ctx->d_cmd.offset = cmd.draw_opts.offset;
  ctx->d_cmd.instances = cmd.draw_opts.instances;

  if (cmd.on_render) {
    ctx->d_cmd.on_render = [ctx, on_render=cmd.on_render]() { on_render(ctx); };
  } else {
    ctx->d_cmd.on_render = {};
  }

  for (const auto& buff : cmd.buffers) {
    auto* ptr = ctx->frame_arena.construct<r_buffer_binding>(buff);
    ctx->d_cmd.buffers.emplace_back(ptr);
  }

  for (const auto& tex : cmd.textures) {
    auto* ptr = ctx->frame_arena.construct<texture_binding>(tex.texture, tex.location);
    ctx->d_cmd.textures.emplace_back(ptr);
  }

  for (const auto& unif : cmd.uniforms) {
    auto* data = ctx->frame_arena.allocate(unif.data.size, unif.data.alignment);
    std::memcpy(data, unif.data.data, unif.data.size);

    auto* desc = ctx->frame_arena.construct<uniform_descriptor>(unif.data.type,
                                                                unif.uniform->location,
                                                                data, unif.data.size);
    ctx->d_cmd.uniforms.emplace_back(desc);
  }

  auto* lcmd = ctx->frame_arena.construct<draw_command>(std::move(ctx->d_cmd));
  ctx->d_list->cmds.emplace_back(lcmd);
  ctx->d_cmd = {};
}

} // namespace
