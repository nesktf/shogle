#include "./internal/platform.hpp"
#include "./internal/opengl/context.hpp"

#define DEF_NODE_OPS(_type, _list, ...) \
void context_t_::insert_node(_type res) { \
  NTF_ASSERT(res && res->ctx == this); \
  res->next = _list; \
  if (_list) { \
    _list->prev = res; \
  } \
  _list = res; \
  __VA_OPT__(++) __VA_ARGS__ \
} \
void context_t_::remove_node(_type res) { \
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

namespace ntf::render {

static constexpr size_t INITIAL_ARENA_PAGE = mibs(16u);

context_t_::context_t_(ctx_alloc::uptr_t<ctx_alloc>&& alloc,
                       ctx_alloc::uptr_t<icontext>&& renderer,
                       ctx_alloc::string_t<char>&& renderer_name,
                       context_api api_,
                       extent2d fbo_ext, fbo_buffer fbo_tbuff,
                       const ctx_render_data::fbo_data_t& fdata) noexcept :
  _alloc{std::move(alloc)},
  _renderer{std::move(renderer)},
  _renderer_name{std::move(renderer_name)},
  _api{api_},
  _default_fbo{this, fbo_ext, fbo_tbuff, fdata},
  _fbo_list_sz{1u},
  _buff_list{nullptr}, _tex_list{nullptr}, _shad_list{nullptr},
  _fbo_list{nullptr}, _pip_list{nullptr} {}

context_t_::~context_t_() noexcept {}

DEF_NODE_OPS(buffer_t, _buff_list);
DEF_NODE_OPS(texture_t, _tex_list);
DEF_NODE_OPS(framebuffer_t, _fbo_list, _fbo_list_sz;);
DEF_NODE_OPS(shader_t, _shad_list);
DEF_NODE_OPS(pipeline_t, _pip_list);

auto context_t_::on_destroy() -> ctx_alloc::uptr_t<ctx_alloc> {
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

static const char* ctx_str(context_api api) {
  switch (api) {
    case context_api::opengl: return "OpenGL";
    case context_api::vulkan: return "Vulkan";
    case context_api::software: return "Software";
    case context_api::none: return "None";
  }
  NTF_UNREACHABLE();
}

expect<context_t> create_context(const context_params& params) {
  auto alloc = ctx_alloc::make_alloc(params.alloc, INITIAL_ARENA_PAGE);
  RET_ERROR_IF(!alloc, "Failed to create allocator");

  auto construct_local_context = [&](ctx_alloc::uptr_t<icontext>&& renderer) -> expect<context_t> {
    auto ctx_name = renderer->get_name(*alloc);
    context_t ctx = alloc->allocate_uninited<context_t_>();
    RET_ERROR_IF(!ctx, "Failed to allocate context");

    const ctx_render_data::fbo_data_t fdata {
      .clear_color = params.fb_clear_color,
      .viewport = params.fb_viewport,
      .clear_flags = params.fb_clear_flags,
    };

    extent2d fext{0, 0};
    fbo_buffer ftbuff = fbo_buffer::depth24u_stencil8u;
    u32 fmsaa = 0;
    renderer->get_dfbo_params(fext, ftbuff, fmsaa);
    NTF_UNUSED(fmsaa); // TODO: come back when we implement multisampled framebuffers

    std::construct_at(ctx,
                      std::move(alloc), std::move(renderer), std::move(ctx_name),
                      params.ctx_api, fext, ftbuff, fdata);
    RENDER_DBG_LOG("Context constructed: {}, {}",
                   ctx_str(params.ctx_api),
                   ctx->name());
    return ctx;
  };

  try {
    switch (params.ctx_api) {
      case context_api::opengl: {
        NTF_ASSERT(params.ctx_params);
        const auto& gl_param = *static_cast<const context_gl_params*>(params.ctx_params);
        return gl_context::load_context(*alloc, gl_param).and_then(construct_local_context);
        break;
      }
      case context_api::software: [[fallthrough]];
      case context_api::vulkan:   [[fallthrough]];
      case context_api::none: {
        NTF_ASSERT(false, "Renderer not implemented");
        break;
      }
    };
  }
  RET_ERROR_CATCH("Failed to create context");
  NTF_UNREACHABLE();
}

void destroy_context(context_t ctx) noexcept {
  if (!ctx) {
    return;
  }

  auto alloc = ctx->on_destroy();
  alloc->destroy(ctx);
  RENDER_DBG_LOG("Context destroyed");
}

void start_frame(context_t ctx) {
  if (!ctx) {
    return;
  }

  ctx->for_each_fbo([](framebuffer_t fbo) {
    fbo->cmds.clear();
  }); 
  ctx->alloc().arena_clear();
}

void end_frame(context_t ctx) {
  if (!ctx) {
    return;
  }
  auto& alloc = ctx->alloc();

  const size_t fbo_count = ctx->fbo_count();
  auto* draw_data = alloc.arena_allocate_uninited<ctx_render_data>(fbo_count);
  size_t fbos_to_blit = 0u;
  auto cmd_sort = [](const ctx_render_cmd& a, const ctx_render_cmd& b) -> bool {
    // Sort by group, then by pipeline index
    const uint32 pip_a = static_cast<uint32>(a.pip);
    const uint32 pip_b = static_cast<uint32>(b.pip);
    return (a.sort_group < b.sort_group || (a.sort_group == b.sort_group && (pip_a < pip_b)));
  };
  ctx->for_each_fbo([&](framebuffer_t fbo) {
    std::sort(fbo->cmds.begin(), fbo->cmds.end(), cmd_sort);
    std::construct_at(draw_data+fbos_to_blit,
                      fbo->handle, fbo->fdata,
                      cspan<ctx_render_cmd>{fbo->cmds.data(), fbo->cmds.size()});
    ++fbos_to_blit;
  });
  if (fbos_to_blit) {
    ctx->renderer().submit_render_data(ctx, cspan<ctx_render_data>{draw_data, fbos_to_blit});
  }

  ctx->renderer().swap_buffers();
}

void device_wait(context_t ctx) {
  if (!ctx) {
    return;
  }
  ctx->renderer().device_wait();
}

void submit_external_command(context_t ctx, const external_cmd& cmd) {
  if (!ctx) {
    return;
  }

  auto target = cmd.target;
  NTF_ASSERT(cmd.render_callback);
  target->cmds.emplace_back(cmd.render_callback);
  auto& tcmd = target->cmds.back();

  if (cmd.state) {
    tcmd.external = *cmd.state;
  }
  tcmd.sort_group = cmd.sort_group;
  tcmd.pip = CTX_HANDLE_TOMB; // for sorting, draw last
}

void submit_render_command(context_t ctx, const render_cmd& cmd) {
  if (!ctx) {
    return;
  }

  auto& alloc = ctx->alloc();

  cmd.target->cmds.emplace_back(cmd.render_callback);
  auto& tcmd = cmd.target->cmds.back();

  NTF_ASSERT(cmd.pipeline);
  tcmd.pip = cmd.pipeline->handle;
  tcmd.opts = cmd.opts;
  tcmd.sort_group = cmd.sort_group;

  NTF_ASSERT(!cmd.buffers.vertex.empty());
  for (auto& buff : tcmd.vbo) {
    buff = CTX_HANDLE_TOMB;
  }
  for (const auto& buff : cmd.buffers.vertex) {
    NTF_ASSERT(buff.layout < ctx_render_cmd::MAX_LAYOUT_NUMBER);
    tcmd.vbo[buff.layout] = buff.buffer->handle;
  }

  if (cmd.buffers.index) {
    tcmd.ebo = cmd.buffers.index->handle;
  } else {
    tcmd.ebo = CTX_HANDLE_TOMB;
  }

  if (!cmd.buffers.shader.empty()) {
    const size_t buff_count = cmd.buffers.shader.size();
    auto* buffers = alloc.arena_allocate_uninited<ctx_render_cmd::shad_bind_t>(buff_count);
    for (size_t i = 0u; const auto& buff : cmd.buffers.shader) {
      NTF_ASSERT(buff.buffer);
      std::construct_at(buffers+i,
                        buff.buffer->handle, buff.binding, buff.offset, buff.size);
      ++i;
    }
    tcmd.shader_buffers = {buffers, buff_count};
  } else {
    tcmd.shader_buffers = {};
  }

  if (!cmd.textures.empty()) {
    const size_t tex_count = cmd.textures.size();
    auto* textures = alloc.arena_allocate_uninited<ctx_tex>(tex_count);
    for (size_t i = 0u; texture_t tex : cmd.textures) {
      NTF_ASSERT(tex);
      std::construct_at(textures+i, tex->handle);
      ++i;
    }
    tcmd.textures = {textures, tex_count};
  } else {
    tcmd.textures = {};
  }

  if (!cmd.consts.empty()) {
    const size_t uniform_count = cmd.consts.size();
    auto* unifs = alloc.arena_allocate_uninited<ctx_render_cmd::unif_const_t>(uniform_count);
    for (size_t i = 0u; const auto& unif : cmd.consts) {
      NTF_ASSERT(unif.uniform);
      const size_t data_sz = unif.size;
      const size_t data_align = unif.alignment;
      auto* data = alloc.arena_allocate(data_sz, data_align);
      std::memcpy(data, unif.data, data_sz);
      std::construct_at(unifs+i,
                        unif.uniform->handle, data, unif.uniform->type, data_sz);
      ++i;
    }
    tcmd.uniforms = {unifs, uniform_count};
  } else {
    tcmd.uniforms = {};
  }
}

context_api get_api(context_t ctx) {
  if (!ctx) {
    return context_api::none;
  }
  return ctx->api();
}

cstring_view<char> get_name(context_t ctx) {
  if (!ctx) {
    return {};
  }
  return ctx->name();
}

} // namespace ntf::render
