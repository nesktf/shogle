#include "./context.hpp"
#include <ntfstl/utility.hpp>

namespace ntf::render {

void gl_context::debug_callback(GLenum src, GLenum type, GLuint id, GLenum severity,
                                GLsizei, const GLchar* msg, const void*) {
  // auto* ctx = reinterpret_cast<gl_context*>(const_cast<void*>(user_ptr));

  std::string_view severity_msg = [severity]() {
    switch (severity) {
      case GL_DEBUG_SEVERITY_HIGH:          return "HIGH";
      case GL_DEBUG_SEVERITY_MEDIUM:        return "MEDIUM";
      case GL_DEBUG_SEVERITY_LOW:           return "LOW";
      case GL_DEBUG_SEVERITY_NOTIFICATION:  return "NOTIFICATION";

      default: break;
    }
    return "UNKNOWN_SEVERITY";
  }();

  std::string_view type_msg = [type]() {
    switch (type) {
      case GL_DEBUG_TYPE_ERROR:               return "ERROR";
      case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
      case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "UNDEFINED_BEHAVIOR";
      case GL_DEBUG_TYPE_PORTABILITY:         return "PORTABILITY";
      case GL_DEBUG_TYPE_PERFORMANCE:         return "PERFORMANCE";
      case GL_DEBUG_TYPE_MARKER:              return "MARKER";
      case GL_DEBUG_TYPE_PUSH_GROUP:          return "PUSH_GROUP";
      case GL_DEBUG_TYPE_POP_GROUP:           return "POP_GROUP";
      case GL_DEBUG_TYPE_OTHER:               return "OTHER";

      default: break;
    };
    return "UNKNOWN_TYPE";
  }();

  std::string_view src_msg = [src]() {
    switch (src) {
      case GL_DEBUG_SOURCE_API:             return "API";
      case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "WINDOW_SYSTEM";
      case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER_COMPILER";
      case GL_DEBUG_SOURCE_THIRD_PARTY:     return "THIRD_PARTY";
      case GL_DEBUG_SOURCE_APPLICATION:     return "APPLICATION";
      case GL_DEBUG_SOURCE_OTHER:           return "OTHER";

      default: break;
    };
    return "UNKNOWN_SOURCE";
  }();

  if (type == GL_DEBUG_TYPE_ERROR) {
    RENDER_ERROR_LOG("[GL_DEBUG][{}][{}][{}][{}] {}",
                     severity_msg, type_msg, src_msg, id, msg);
  } else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
    RENDER_VRB_LOG("[GL_DEBUG][{}][{}][{}][{}] {}",
                   severity_msg, type_msg, src_msg, id, msg);
  } else {
    RENDER_DBG_LOG("[GL_DEBUG][{}][{}][{}][{}] {}",
                   severity_msg, type_msg, src_msg, id, msg);
  }
}

expect<ctx_alloc::uptr_t<icontext>> gl_context::load_context(ctx_alloc& alloc,
                                                             const context_gl_params& params) {
  NTF_ASSERT(params.get_proc_address);
  NTF_ASSERT(params.make_current);
  NTF_ASSERT(params.swap_buffers);

  // hack
  // doesn't matter anyways, glad uses global state
  // (TODO: Store the gl functions in the context or state object)
  static void* glad_gl_ctx;
  static void* (*glad_gl_proc)(void*, const char*);
  glad_gl_ctx = params.gl_ctx;
  glad_gl_proc = params.get_proc_address;
  bool glad_succ = gladLoadGLLoader(+[](const char* name) -> void* {
    // RENDER_DBG_LOG("{}", name);
    return std::invoke(glad_gl_proc, glad_gl_ctx, name);
  });
  glad_gl_ctx = nullptr;
  glad_gl_proc = nullptr;
  RET_ERROR_IF(!glad_succ, "Failed to load GLAD");
  icontext* ctx = alloc.construct<gl_context>(alloc, params);
  NTF_ASSERT(ctx);
  return alloc.wrap_unique(ctx);
}

gl_context::gl_context(ctx_alloc& alloc, const context_gl_params& params) noexcept :
  _alloc{alloc}, _funcs{params},
  _state{alloc}, _vao{},
  _buffers{alloc}, _textures{alloc}, _shaders{alloc}, _programs{alloc}, _framebuffers{alloc}
{
  _state.init(gl_context::debug_callback, this);
  _state.create_vao(_vao);
  _state.vao_bind(_vao.id);
  RENDER_DBG_LOG("OpenGL context created");
}

gl_context::~gl_context() noexcept {
  RENDER_DBG_LOG("OpenGL context destroyed");
}

void gl_context::get_limits(ctx_limits& limits) {
  GLint max_tex;
  GL_CALL(glGetIntegerv, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_tex);

  GLint max_tex_lay;
  GL_CALL(glGetIntegerv, GL_MAX_ARRAY_TEXTURE_LAYERS, &max_tex_lay);
  limits.tex_max_layers = max_tex_lay;

  GLint max_tex_dim;
  GL_CALL(glGetIntegerv, GL_MAX_TEXTURE_SIZE, &max_tex_dim);
  limits.tex_max_extent = max_tex_dim;

  GLint max_tex_dim3d;
  GL_CALL(glGetIntegerv, GL_MAX_3D_TEXTURE_SIZE, &max_tex_dim3d);
  limits.tex_max_extent3d = max_tex_dim3d;

  // GLint max_fbo_attach;
  // glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_fbo_attach);
  // _fbo_max_attachments = max_fbo_attach;
}

ctx_alloc::string_t<char> gl_context::get_name(ctx_alloc& alloc) {
  const char* name_str = reinterpret_cast<const char*>(GL_CALL_RET(glGetString, GL_RENDERER));
  const char* vendor_str = reinterpret_cast<const char*>(GL_CALL_RET(glGetString, GL_VENDOR));
  const char* ver_str = reinterpret_cast<const char*>(GL_CALL_RET(glGetString, GL_VERSION));
  return alloc.fmt_string("{} [{} - {}]", name_str, vendor_str, ver_str);
}

void gl_context::submit_render_data(context_t ctx, cspan<ctx_render_data> render_data) {
  std::invoke(_funcs.make_current, _funcs.gl_ctx);
  _state.vao_bind(_vao.id);

  auto render_command = [this](const ctx_render_cmd& cmd) {
    // Bind program
    NTF_ASSERT(_programs.validate(cmd.pip));
    auto& prog = _programs.get(cmd.pip);
    NTF_ASSERT(prog.id);
    _state.program_prepare_state(prog);

    // Bind vertex attributes and buffers
    NTF_ASSERT(!prog.layout.empty());
    for (const auto& attr : prog.layout) {
      NTF_ASSERT(attr.location < ctx_render_cmd::MAX_LAYOUT_NUMBER);
      const ctx_buff buff = cmd.vbo[attr.location];
      NTF_ASSERT(buff != CTX_HANDLE_TOMB);
      const auto& vbo = _buffers.get(buff);
      _state.attribute_bind(attr, vbo);
    }

    // Bind index buffer (if any)
    if (_buffers.validate(cmd.ebo)) {
      auto& ebo = _buffers.get(cmd.ebo);
      NTF_ASSERT(ebo.type == GL_ELEMENT_ARRAY_BUFFER);
      _state.buffer_bind(ebo.id, ebo.type);
    }

    // Bind uniform & shader buffers
    for (const auto& buffer : cmd.shader_buffers) {
      const auto& gl_buff = _buffers.get(buffer.handle);
      NTF_ASSERT(gl_buff.type == GL_SHADER_STORAGE_BUFFER || gl_buff.type == GL_UNIFORM_BUFFER);
      NTF_ASSERT(buffer.offset+buffer.size <= gl_buff.size);
      GL_CALL(glBindBufferRange,
              gl_buff.type, buffer.binding, gl_buff.id, buffer.offset, buffer.size);
    }

    // Bind textures, set the sampler index in order
    for (uint32 index = 0u; const ctx_tex tex_handle : cmd.textures) {
      auto& tex = _textures.get(tex_handle);
      _state.texture_bind(tex.id, tex.type, index);
      ++index;
    }

    // Upload uniforms, if any
    for (const auto& unif : cmd.uniforms) {
      _state.push_uniform(static_cast<GLuint>(unif.location), unif.type, unif.data);
    }

    // Draw things
    const auto& draw_opts = cmd.opts;
    NTF_ASSERT(draw_opts.vertex_count);
    if (_buffers.validate(cmd.ebo)) {
      const void* idx_offset =
        reinterpret_cast<const void*>(draw_opts.index_offset*sizeof(uint32));

      const GLenum format = GL_UNSIGNED_INT;
      if (draw_opts.instances) {
        GL_CALL(glDrawElementsInstancedBaseVertex,
                prog.primitive, draw_opts.vertex_count, format,
                idx_offset, draw_opts.instances, draw_opts.vertex_offset);
      } else {
        GL_CALL(glDrawElementsBaseVertex,
                prog.primitive, draw_opts.vertex_count, format,
                idx_offset, draw_opts.vertex_offset);
      }
    } else {
      if (draw_opts.instances) {
        GL_CALL(glDrawArraysInstanced,
                prog.primitive, draw_opts.vertex_offset, draw_opts.vertex_count,
                draw_opts.instances);
      } else {
        GL_CALL(glDrawArrays,
                prog.primitive, draw_opts.vertex_offset, draw_opts.vertex_count);
      }
    }
  };

  for (const auto& data : render_data) {
    auto target = data.target;
    auto fdata = data.data;
    const auto fbo_id =
      _framebuffers.validate(target) ? _framebuffers.get(target).id : gl_state::DEFAULT_FBO;
    _state.framebuffer_prepare_state(fbo_id, fdata->clear_flags,
                                     fdata->viewport, fdata->clear_color);

    for (const auto& cmd : data.commands) {
      bool external_render = false;
      std::visit(overload {
        [&](function_view<void(context_t)> on_render) {
          if (on_render) {
            std::invoke(on_render, ctx);
          }
        },
        [&](function_view<void(context_t, ctx_handle)> on_render) {
          NTF_ASSERT(on_render);
          if (cmd.external) {
            _state.prepare_state(*cmd.external);
          }
          std::invoke(on_render, ctx, static_cast<ctx_handle>(fbo_id));
          external_render = true;
        },
      }, cmd.on_render);
      if (!external_render) {
        render_command(cmd);
      }
    }
  }
}

void gl_context::device_wait() {}

void gl_context::swap_buffers() {
  std::invoke(_funcs.swap_buffers, _funcs.gl_ctx);
}

} // namespace ntf::render
