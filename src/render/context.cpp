#include "./internal/common.hpp"
#include "./internal/opengl/context.hpp"

#define DEF_NODE_OPS(_type, _list, ...) \
void r_context_::insert_node(_type res) { \
  NTF_ASSERT(res && res->ctx == this); \
  res->next = _list; \
  if (_list) { \
    _list->prev = res; \
  } \
  _list = res; \
  __VA_OPT__(++) __VA_ARGS__ \
} \
void r_context_::remove_node(_type res) { \
  NTF_ASSERT(res && res->ctx == this); \
  if (!res->prev) { \
    NTF_ASSERT(res == _list); \
    if (_list->next) { \
      _list->next->prev = nullptr; \
    } \
    _list = _list->next; \
  } else { \
    if (res->next) { \
      res->next->prev = res->prev; \
    } \
    res->prev->next = res->next; \
  } \
  res->next = nullptr; \
  res->prev = nullptr; \
  __VA_OPT__(--) __VA_ARGS__ \
}

namespace ntf {

static constexpr size_t INITIAL_ARENA_PAGE = mibs(16u);

static r_allocator base_alloc {
  .user_ptr = nullptr,
  .mem_alloc = +[](void*, size_t sz, size_t) -> void* { return std::malloc(sz); },
  .mem_free  = +[](void*, void* mem, size_t) -> void { std::free(mem); }
};

auto rp_alloc::make_alloc(weak_cref<r_allocator> alloc_in, size_t arena_size) -> uptr_t<rp_alloc> {
  auto& alloc = alloc_in ? *alloc_in : base_alloc;
  auto* ptr = static_cast<rp_alloc*>(std::invoke(alloc.mem_alloc, alloc.user_ptr,
                                                 sizeof(rp_alloc), alignof(rp_alloc)));
  if (ptr) {
    std::construct_at(ptr, alloc);
    if (!ptr->arena_init(arena_size)) {
      RENDER_ERROR_LOG("Failed to init allocator arena ({} bytes)", arena_size);
      std::destroy_at(ptr);
      std::invoke(alloc.mem_free, alloc.user_ptr, ptr, sizeof(rp_alloc));
      ptr = nullptr;
    } else {
      SHOGLE_LOG(verbose, "[ntf::r_context] Allocator inited with arena of {} bytes)", arena_size);
    }
  }
  return uptr_t<rp_alloc>{ptr, alloc_del_t<rp_alloc>{alloc.user_ptr, alloc.mem_free}};
}

r_context_::r_context_(rp_alloc::uptr_t<rp_alloc>&& alloc,
                       rp_alloc::uptr_t<rp_context>&& renderer,
                       r_window win, r_api api_,
                       extent2d fbo_ext, r_test_buffer fbo_tbuff,
                       const rp_fbo_frame_data& fdata) noexcept :
  _alloc{std::move(alloc)}, _renderer{std::move(renderer)},
  _api{api_}, _win{win},
  _default_fbo{this, fbo_ext, fbo_tbuff, fdata},
  _fbo_list_sz{1u},
  _buff_list{nullptr}, _tex_list{nullptr}, _shad_list{nullptr},
  _fbo_list{nullptr}, _pip_list{nullptr} {}

r_context_::~r_context_() noexcept {}

DEF_NODE_OPS(r_buffer, _buff_list);
DEF_NODE_OPS(r_texture, _tex_list);
DEF_NODE_OPS(r_framebuffer, _fbo_list, _fbo_list_sz;);
DEF_NODE_OPS(r_shader, _shad_list);
DEF_NODE_OPS(r_pipeline, _pip_list);

auto r_context_::on_destroy() -> rp_alloc::uptr_t<rp_alloc> {
  auto* pip = _pip_list;
  while (pip) {
    pip = pip->next;
    _renderer->destroy_pipeline(pip->handle);
    _alloc->destroy(pip);
  }

  auto* fbo = _fbo_list;
  while (fbo) {
    fbo = fbo->next;
    _renderer->destroy_framebuffer(fbo->handle);
    _alloc->destroy(fbo);
  }

  auto* shad = _shad_list;
  while (shad) {
    shad = shad->next;
    _renderer->destroy_shader(shad->handle);
    _alloc->destroy(shad);
  }

  auto* tex = _tex_list;
  while (tex) {
    tex = tex->next;
    _renderer->destroy_texture(tex->handle);
    _alloc->destroy(tex);
  }

  auto* buff = _buff_list;
  while (buff) {
    buff = buff->next;
    _renderer->destroy_buffer(buff->handle);
    _alloc->destroy(buff);
  }

  return std::move(_alloc);
}


static r_expected<rp_alloc::uptr_t<rp_context>> load_platform_ctx(
  rp_alloc& alloc, r_api api, r_window win, uint32 swap_interval
) {
  RET_ERROR_IF(!win, "Invalid window handle");
  rp_context* ctx = nullptr;
  try {
    switch (api) {
      case r_api::opengl: {
        SHOGLE_GL_MAKE_CTX_CURRENT(win);
        SHOGLE_GL_SET_SWAP_INTERVAL(win, static_cast<int>(swap_interval));
        RET_ERROR_IF(!gladLoadGLLoader(SHOGLE_GL_LOAD_PROC),
                     "Failed to load GLAD");
        ctx = alloc.construct<gl_context>(alloc, win, swap_interval);
        break;
      }
      default: {
        RET_ERROR("Renderer not implemented");
        break;
      }
    }
  }
  RET_ERROR_CATCH("Failed to load platform context");
  RET_ERROR_IF(!ctx, "Failed to allocate platform context");

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

  return alloc.wrap_unique(ctx);
}

r_expected<r_context> r_create_context(const r_context_params& params) {
  auto alloc = rp_alloc::make_alloc(params.alloc, INITIAL_ARENA_PAGE);
  RET_ERROR_IF(!alloc, "Failed to create allocator");

  return load_platform_ctx(*alloc, params.renderer_api, params.window, params.swap_interval)
  .and_then([&](rp_alloc::uptr_t<rp_context>&& renderer) -> r_expected<r_context> {
    r_context ctx = alloc->allocate_uninited<r_context_>();
    RET_ERROR_IF(!ctx, "Failed to allocate context");

    const rp_fbo_frame_data fdata {
      .clear_color = params.fb_color,
      .viewport = params.fb_viewport,
      .clear_flags = params.fb_clear,
    };
    extent2d fext{0, 0}; // TODO: Get this things from the platform context
    r_test_buffer ftbuff{r_test_buffer::depth24u_stencil8u};

    std::construct_at(ctx,
                      std::move(alloc), std::move(renderer),
                      params.window, params.renderer_api, fext, ftbuff, fdata);

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

  auto alloc = ctx->on_destroy();
  alloc->destroy(ctx);
  alloc->arena_destroy();
  SHOGLE_LOG(debug, "[ntf::r_context][DESTROY]");
}

void r_start_frame(r_context ctx) {
  if (!ctx) {
    return;
  }

  ctx->for_each_fbo([](r_framebuffer_& fbo) {
    fbo.cmds.clear();
  }); 
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
  auto& alloc = ctx->alloc();

  const size_t fbo_count = ctx->fbo_count();
  auto* draw_data = alloc.arena_allocate_uninited<rp_draw_data>(fbo_count);
  size_t fbos_to_blit = 0u;
  ctx->for_each_fbo([&](r_framebuffer_& fbo) {
    // TODO: Sort the draw commands before submiting here
    std::construct_at(draw_data+fbos_to_blit,
                      fbo.handle, fbo.fdata,
                      cspan<rp_draw_cmd>{fbo.cmds.data(), fbo.cmds.size()});
    ++fbos_to_blit;
  });
  if (fbos_to_blit) {
    ctx->renderer().submit(ctx, cspan<rp_draw_data>{draw_data, fbos_to_blit});
  }

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

  cmd.target->cmds.emplace_back(); // TODO: Use the allocator here too
  auto& d_cmd = cmd.target->cmds.back();

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
      ++i;
    }
    d_cmd.uniforms = {unifs, uniform_count};
  }
}

} // namespace
