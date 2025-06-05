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
    SHOGLE_LOG(error, "[ntf::gl_context][GL_DEBUG][{}][{}][{}][{}] {}",
               severity_msg, type_msg, src_msg, id, msg);
  } else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
    SHOGLE_LOG(verbose, "[ntf::gl_context][GL_DEBUG][{}][{}][{}][{}] {}",
               severity_msg, type_msg, src_msg, id, msg);
  } else {
    SHOGLE_LOG(debug, "[ntf::gl_context][GL_DEBUG][{}][{}][{}][{}] {}",
               severity_msg, type_msg, src_msg, id, msg);
  }
}

gl_context::gl_context(ctx_alloc& alloc, window_t win, uint32 swap_interval) noexcept :
  _alloc{alloc}, _win{win}, _swap_interval{swap_interval},
  _state{alloc}, _vao{},
  _buffers{alloc}, _textures{alloc}, _shaders{alloc}, _programs{alloc}, _framebuffers{alloc}
{
  _state.init(gl_state::init_data_t{
    .dbg = gl_context::debug_callback,
    .ctx = this,
  });
  _state.create_vao(_vao);
  _state.bind_vao(_vao.id);
  SHOGLE_LOG(verbose, "[ntf::gl_context] OpenGL context created");
}

gl_context::~gl_context() noexcept {
  SHOGLE_LOG(verbose, "[ntf::gl_context] OpenGL context destroyed");
}

void gl_context::get_meta(ctx_meta& meta) {
  meta.api = context_api::opengl;
  _state.get_limits(meta.limits);
  const char* name_str = reinterpret_cast<const char*>(GL_CALL_RET(glGetString, GL_RENDERER));
  const char* vendor_str = reinterpret_cast<const char*>(GL_CALL_RET(glGetString, GL_VENDOR));
  const char* ver_str = reinterpret_cast<const char*>(GL_CALL_RET(glGetString, GL_VERSION));
  meta.name_str = _alloc.fmt_string_args("{} [{} - {}]", name_str, vendor_str, ver_str);
}

ctx_buff_status gl_context::create_buffer(ctx_buff& buff, const ctx_buff_desc& desc) {
  ctx_buff handle = _buffers.acquire();
  NTF_ASSERT(check_handle(handle));
  auto& buffer = _buffers.get(handle);
  const auto status = _state.create_buffer(buffer, desc.type, desc.flags, desc.size, desc.data);
  if (status != CTX_BUFF_STATUS_OK) {
    _buffers.push(handle);
    return status;
  }

  NTF_ASSERT(buffer.id);
  buff = handle;
  return status;
}

ctx_buff_status gl_context::destroy_buffer(ctx_buff buff) noexcept {
  if (!_buffers.validate(buff)) {
    return CTX_BUFF_STATUS_INVALID_HANDLE;
  }
  auto& buffer = _buffers.get(buff);
  _state.destroy_buffer(buffer);
  _buffers.push(buff);
  return CTX_BUFF_STATUS_OK;
}

ctx_buff_status gl_context::update_buffer(ctx_buff buff, const buffer_data& data) {
  if (!_buffers.validate(buff)) {
    return CTX_BUFF_STATUS_INVALID_HANDLE;
  }
  auto& buffer = _buffers.get(buff);
  return _state.buffer_upload(buffer, data.size, data.offset, data.data);
}

ctx_buff_status gl_context::map_buffer(ctx_buff buff, void** ptr, size_t size, size_t offset) {
  if (!_buffers.validate(buff)) {
    return CTX_BUFF_STATUS_INVALID_HANDLE;
  }
  auto& buffer = _buffers.get(buff);
  return _state.buffer_map(buffer, ptr, size, offset);
}

ctx_buff_status gl_context::unmap_buffer(ctx_buff buff, void* ptr) noexcept {
  if (!_buffers.validate(buff) || !ptr) {
    return CTX_BUFF_STATUS_INVALID_HANDLE;
  }
  auto& buffer = _buffers.get(buff);
  _state.buffer_unmap(buffer, ptr);
  return CTX_BUFF_STATUS_OK;
}

ctx_tex_status gl_context::create_texture(ctx_tex& tex, const ctx_tex_desc& desc) {
  ctx_tex handle = _textures.acquire();
  NTF_ASSERT(check_handle(handle));
  auto& texture = _textures.get(handle);
  const auto status = _state.create_texture(texture, desc.type, desc.format, 
                                            desc.sampler, desc.addressing,
                                            desc.extent, desc.layers, desc.levels);
  if (status != CTX_TEX_STATUS_OK) {
    _textures.push(handle);
    return status;
  }
  NTF_ASSERT(texture.id);
  tex = handle;
  return status;
}

ctx_tex_status gl_context::destroy_texture(ctx_tex tex) noexcept {
  if (!_textures.validate(tex)) {
    return CTX_TEX_STATUS_INVALID_HANDLE;
  }
  auto& texture = _textures.get(tex);
  _state.destroy_texture(texture);
  _textures.push(tex);
  return CTX_TEX_STATUS_OK;
}

ctx_tex_status gl_context::update_texture(ctx_tex tex, const ctx_tex_data& data) {
  if (!_textures.validate(tex)) {
    return CTX_TEX_STATUS_INVALID_HANDLE;
  }
  auto& texture = _textures.get(tex);
  for (const auto& image : data.images) {
    auto status = _state.texture_upload(texture, image.bitmap, image.format,
                                        image.alignment, image.offset,
                                        image.layer, image.level);
    if (status != CTX_TEX_STATUS_OK){
      return status;
    }
  }
  if (data.generate_mipmaps) {
    if (texture.levels < 2) {
      return CTX_TEX_STATUS_INVALID_LEVELS;
    }
    _state.texture_gen_mipmaps(texture);
  }
  return CTX_TEX_STATUS_OK;
}

ctx_tex_status gl_context::update_texture(ctx_tex tex, const ctx_tex_opts& opts) {
  if (!_textures.validate(tex)) {
    return CTX_TEX_STATUS_INVALID_HANDLE;
  }
  auto& texture = _textures.get(tex);
  bool add_succ = _state.texture_set_addressing(texture, opts.addresing);
  bool sam_succ = _state.texture_set_sampler(texture, opts.sampler);
  if (!add_succ) {
    return CTX_TEX_STATUS_INVALID_ADDRESING;
  }
  if (!sam_succ) {
    return CTX_TEX_STATUS_INVALID_SAMPLER;
  }
  return CTX_TEX_STATUS_OK;
}

ctx_shad_status gl_context::create_shader(ctx_shad& shad, shad_err_str& err,
                                          const ctx_shad_desc& desc) {
  ctx_shad handle = _shaders.acquire();
  NTF_ASSERT(check_handle(handle));
  auto& shader = _shaders.get(handle);
  const auto status = _state.create_shader(shader, desc.type, desc.source, err);
  if (status != CTX_SHAD_STATUS_OK) {
    _shaders.push(handle);
    return status;
  }
  NTF_ASSERT(shader.id);
  shad = handle;
  return status;
}

ctx_shad_status gl_context::destroy_shader(ctx_shad shad) noexcept {
  if (!_shaders.validate(shad)) {
    return CTX_SHAD_STATUS_INVALID_HANDLE;
  }
  auto& shader = _shaders.get(shad);
  _state.destroy_shader(shader);
  _shaders.push(shad);
  return CTX_SHAD_STATUS_OK;
}

ctx_pip_status gl_context::create_pipeline(ctx_pip& pip, pip_err_str& err,
                                           const ctx_pip_desc& desc){
  NTF_ASSERT(!desc.stages.empty());
  auto shads = _alloc.arena_span<gl_state::shader_t*>(desc.stages.size());
  NTF_ASSERT(!shads.empty())
  for (size_t i = 0u; const ctx_shad stage : desc.stages) {
    auto& shader = _shaders.get(stage);
    shads[i] = &shader;
    ++i;
  }

  ctx_pip handle = _programs.acquire();
  NTF_ASSERT(check_handle(handle));
  auto& program = _programs.get(handle);
  const auto status = _state.create_program(program, shads, desc.primitive,
                                            desc.poly_mode, desc.poly_width,
                                            desc.tests, err);
  if (status != CTX_PIP_STATUS_OK) {
    _programs.push(handle);
    return status;
  }
  NTF_ASSERT(program.id);
  pip = handle;
  return status;
}

ctx_pip_status gl_context::destroy_pipeline(ctx_pip pip) noexcept {
  if (!_programs.validate(pip)) {
    return CTX_PIP_STATUS_INVALID_HANDLE;
  }
  auto& program = _programs.get(pip);
  _state.destroy_program(program);
  _programs.push(pip);
  return CTX_PIP_STATUS_OK;
}

ctx_fbo_status gl_context::create_framebuffer(ctx_fbo& fbo, const ctx_fbo_desc& desc) {
  NTF_ASSERT(!desc.attachments.empty());
  auto atts = _alloc.arena_span<gl_state::fbo_attachment_t>(desc.attachments.size());
  for (size_t i = 0u; const auto& att : desc.attachments) {
    auto& tex = _textures.get(att.texture);
    atts[i].tex = &tex;
    atts[i].layer = att.layer;
    atts[i].level = att.level;
    ++i;
  }

  ctx_fbo handle = _framebuffers.acquire();
  NTF_ASSERT(check_handle(handle));
  auto& framebuffer = _framebuffers.get(handle);
  const auto status = _state.create_framebuffer(framebuffer, desc.extent,
                                                desc.test_buffer, atts);
  if (status != CTX_FBO_STATUS_OK) {
    _framebuffers.push(handle);
    return status;
  }
  NTF_ASSERT(framebuffer.id);
  fbo = handle;
  return status;
}

ctx_fbo_status gl_context::destroy_framebuffer(ctx_fbo fbo) noexcept {
  if (!_framebuffers.validate(fbo)) {
    return CTX_FBO_STATUS_INVALID_HANDLE;
  }
  auto& framebuffer = _framebuffers.get(fbo);
  _state.destroy_framebuffer(framebuffer);
  _framebuffers.push(fbo);
  return CTX_FBO_STATUS_OK;
}

void gl_context::submit_render_data(context_t ctx, cspan<ctx_render_data> render_data) {
  SHOGLE_GL_MAKE_CTX_CURRENT(_win);
  _state.bind_vao(_vao.id);

  auto render_command = [this](const ctx_render_cmd& cmd) {
    // Bind vertex buffer
    auto& vbo = _buffers.get(cmd.vbo);
    NTF_ASSERT(vbo.type == GL_ARRAY_BUFFER);
    _state.buffer_bind(vbo.id, vbo.type);

    // Bind program
    NTF_ASSERT(cmd.pip);
    auto& prog = _programs.get(cmd.pip);
    NTF_ASSERT(prog.id);
    _state.program_prepare_state(prog);

    // Bind vertex attributes
    NTF_ASSERT(!prog.layout.empty());
    _state.attribute_bind(prog.layout);

    // Bind index buffer (if any)
    if (check_handle(cmd.ebo)) {
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
      _state.push_uniform(static_cast<GLuint>(unif.handle), unif.type, unif.data);
    }

    // Draw things
    const auto& draw_opts = cmd.opts;
    NTF_ASSERT(draw_opts.vertex_count);
    if (check_handle(cmd.ebo)) {
      const void* offset = reinterpret_cast<const void*>(draw_opts.vertex_offset*sizeof(uint32));
      const GLenum format = GL_UNSIGNED_INT;
      if (draw_opts.instances) {
        GL_CALL(glDrawElementsInstanced,
          prog.primitive, draw_opts.vertex_count, format, offset, draw_opts.instances
        );
      } else {
        GL_CALL(glDrawElements,
          prog.primitive, draw_opts.vertex_count, format, offset
        );
      }
    } else {
      if (draw_opts.instances) {
        GL_CALL(glDrawArraysInstanced,
          prog.primitive, draw_opts.vertex_offset, draw_opts.vertex_count, draw_opts.instances
        );
      } else {
        GL_CALL(glDrawArrays,
          prog.primitive, draw_opts.vertex_offset, draw_opts.vertex_count
        );
      }
    }
  };

  for (const auto& data : render_data) {
    auto target = data.target;
    auto fdata = data.data;
    const auto fbo_id = target ? _framebuffers.get(target).id : gl_state::DEFAULT_FBO;
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
  SHOGLE_GL_SWAP_BUFFERS(_win);
}

} // namespace ntf::render
