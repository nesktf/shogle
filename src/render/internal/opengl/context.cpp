#include "./context.hpp"

namespace ntf {

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

gl_context::gl_context(rp_alloc& alloc, r_window win, uint32 swap_interval) noexcept :
  _alloc{alloc}, _win{win}, _swap_interval{swap_interval},
  _state{}, _vao{},
  _buffers{alloc}, _textures{alloc}, _shaders{alloc}, _programs{alloc}, _framebuffers{alloc}
{
  _state.init(gl_state::init_data_t{
    .dbg = gl_context::debug_callback,
    .ctx = this,
  });
  _vao = _state.create_vao();
  _state.bind_vao(_vao.id);
  SHOGLE_LOG(verbose, "[ntf::gl_context] OpenGL context created");
}

gl_context::~gl_context() noexcept {
  SHOGLE_LOG(verbose, "[ntf::gl_context] OpenGL context destroyed");
}

void gl_context::get_meta(rp_platform_meta& meta) {
  meta.api = r_api::opengl;
  meta.name_str = reinterpret_cast<const char*>(GL_CALL_RET(glGetString, GL_RENDERER));
  meta.vendor_str = reinterpret_cast<const char*>(GL_CALL_RET(glGetString, GL_VENDOR));
  meta.version_str = reinterpret_cast<const char*>(GL_CALL_RET(glGetString, GL_VERSION));

  meta.tex_max_layers = _state.tex_limits().max_layers;
  meta.tex_max_extent = _state.tex_limits().max_dim;
  meta.tex_max_extent3d = _state.tex_limits().max_dim3d;
}

r_platform_buffer gl_context::create_buffer(const rp_buff_desc& desc) {
  gl_state::buffer_t buffer = _state.create_buffer(
    desc.type, desc.flags, desc.size, desc.initial_data
  );
  NTF_ASSERT(buffer.id);
  auto handle = _buffers.acquire();
  _buffers.get(handle) = buffer;
  return handle;
}

void gl_context::update_buffer(r_platform_buffer buf, const rp_buff_data& data) {
  auto& buffer = _buffers.get(buf);
  _state.update_buffer(buffer, data.data, data.len, data.offset);
}

void gl_context::destroy_buffer(r_platform_buffer buf) noexcept {
  auto& buffer = _buffers.get(buf);
  _state.destroy_buffer(buffer);
  _buffers.push(buf);
}

void* gl_context::map_buffer(r_platform_buffer buf, const rp_buff_mapping& map) {
  auto& buffer = _buffers.get(buf);
  return _state.map_buffer(buffer, map.offset, map.len);
}

void gl_context::unmap_buffer(r_platform_buffer buf, void* ptr) noexcept {
  auto& buffer = _buffers.get(buf);
  _state.unmap_buffer(buffer, ptr);
}

r_platform_texture gl_context::create_texture(const rp_tex_desc& desc) {
  gl_state::texture_t texture = _state.create_texture(
    desc.type, desc.format, desc.sampler, desc.addressing,
    desc.extent, desc.layers, desc.levels
  );
  NTF_ASSERT(texture.id);
  if (desc.initial_data) {
    for (const auto& img : desc.initial_data) {
      _state.update_texture_data(
        texture, img.texels, img.format, img.alignment, img.offset, img.layer, img.layer
      );
    }
    if (desc.gen_mipmaps && texture.levels > 1) {
      _state.gen_texture_mipmaps(texture);
    }
  }
  auto handle = _textures.acquire();
  _textures.get(handle) = texture;
  return handle;
}

void gl_context::upload_texture_images(r_platform_texture tex,
                                       cspan<rp_tex_image_data> images, bool regen_mips) {
  auto& texture = _textures.get(tex);
  for (const auto& image : images) {
    _state.update_texture_data(texture,
                               image.texels, image.format, image.alignment,
                               image.offset, image.layer, image.level);
  }
  if (regen_mips) {
    _state.gen_texture_mipmaps(texture);
  }
}
void gl_context::update_texture_options(r_platform_texture tex, const rp_tex_opts& opts) {
  auto& texture = _textures.get(tex);
  _state.update_texture_sampler(texture, opts.sampler);
  _state.update_texture_addressing(texture, opts.addressing);
}

void gl_context::destroy_texture(r_platform_texture tex) noexcept {
  auto& texture = _textures.get(tex);
  _state.destroy_texture(texture);
  _textures.push(tex);
}

r_platform_fbo gl_context::create_framebuffer(const rp_fbo_desc& desc) {
  r_platform_fbo handle;
  if (desc.attachments) {
    NTF_ASSERT(desc.attachments);
    auto atts = _alloc.arena_span<gl_state::fbo_attachment_t>(desc.attachments.size());
    if (!atts) {
      throw ntf::error<>{"Failed to allocate attachment descriptors"};
    }
    for (size_t i = 0u; const auto& att : desc.attachments) {
      auto& tex = _textures.get(att.texture);
      atts[i].tex = &tex;
      atts[i].layer = att.layer;
      atts[i].level = att.level;
      ++i;
    }

    gl_state::framebuffer_t framebuffer = _state.create_framebuffer(
      desc.extent.x, desc.extent.y, desc.test_buffer, atts.data(), atts.size()
    );
    NTF_ASSERT(framebuffer.id);
    handle = _framebuffers.acquire();
    _framebuffers.get(handle) = framebuffer;
  } else {
    NTF_ASSERT(desc.color_buffer);
    gl_state::framebuffer_t framebuffer = _state.create_framebuffer(
      desc.extent.x, desc.extent.y, desc.test_buffer, *desc.color_buffer
    );
    NTF_ASSERT(framebuffer.id);
    handle = _framebuffers.acquire();
    _framebuffers.get(handle) = framebuffer;
  }

  return handle;
}

void gl_context::destroy_framebuffer(r_platform_fbo fb) noexcept {
  auto& framebuffer = _framebuffers.get(fb);
  _state.destroy_framebuffer(framebuffer);
  _framebuffers.push(fb);
}

r_platform_shader gl_context::create_shader(const rp_shad_desc& desc) {
  NTF_ASSERT(!desc.source.empty());
  gl_state::shader_t shader = _state.create_shader(desc.type, desc.source);
  auto handle = _shaders.acquire();
  _shaders.get(handle) = shader;
  return handle;
}

void gl_context::destroy_shader(r_platform_shader shad) noexcept {
  auto& shader = _shaders.get(shad);
  _state.destroy_shader(shader);
  _shaders.push(shad);
}

r_platform_pipeline gl_context::create_pipeline(const rp_pip_desc& desc) {
  NTF_ASSERT(desc.stages);
  auto shads = _alloc.arena_span<gl_state::shader_t*>(desc.stages.size());
  if (!shads) {
    throw ntf::error<>{"Failed to allocate stages descriptors"};
  }
  for (size_t i = 0u; const auto& stage : desc.stages) {
    auto& shader = _shaders.get(stage);
    shads[i] = &shader;
    ++i;
  }
  gl_state::program_t prog = _state.create_program(
    shads.data(), shads.size(), desc.primitive
  );

  NTF_ASSERT(desc.uniforms);
  _state.query_program_uniforms(prog, *desc.uniforms);
  prog.layout = desc.layout.get();

  NTF_ASSERT(prog.id);
  auto handle = _programs.acquire();
  _programs.get(handle) = prog;
  return handle;
}

void gl_context::destroy_pipeline(r_platform_pipeline pipeline) noexcept {
  auto& prog = _programs.get(pipeline);
  _state.destroy_program(prog);
  _programs.push(pipeline);
}

void gl_context::update_pipeline_options(r_pipeline pip, const rp_pip_opts& opts) {
}

void gl_context::submit(r_context ctx, cspan<rp_draw_data> draw_data) {
  SHOGLE_GL_MAKE_CTX_CURRENT(_win);
  _state.bind_vao(_vao.id);
  for (const auto& data : draw_data) {
    auto target = data.target;
    auto fdata = data.fdata;
    _state.prepare_draw_target(target ? _framebuffers.get(target).id : gl_state::DEFAULT_FBO,
                               fdata->clear_flags, fdata->viewport, fdata->clear_color);

    for (const auto& cmd : data.cmds) {
      if (cmd.on_render) {
        cmd.on_render(ctx);
      }

      bool index_buffer = false;
      bool rebind = false;
      for (const auto& buffer : cmd.buffers) {
        const auto& gl_buff = _buffers.get(buffer.buffer);
        if (gl_buff.type == GL_ELEMENT_ARRAY_BUFFER) {
          index_buffer = true;
        }

        if (gl_buff.type == GL_ARRAY_BUFFER) {
          rebind = _state.bind_buffer(gl_buff.id, gl_buff.type);
        } else {
          _state.bind_buffer(gl_buff.id, gl_buff.type);
        }
        if (gl_buff.type == GL_SHADER_STORAGE_BUFFER) {
          if (!buffer.location) {
            SHOGLE_LOG(warning, "[ntf::gl_context::submit] No binding provided for SSBO");
            continue;
          }
          GL_CALL(glBindBufferBase, GL_SHADER_STORAGE_BUFFER, *buffer.location, gl_buff.id);
        }
      }

      NTF_ASSERT(cmd.count);
      NTF_ASSERT(cmd.pipeline);

      const auto& prog = _programs.get(cmd.pipeline);
      NTF_ASSERT(prog.id);

      rebind = (_state.bind_program(prog.id) || rebind);
      if (rebind) {
        auto& layout = prog.layout;
        _state.bind_attributes(
          layout->descriptors.get(), layout->descriptors.size(), layout->stride
        );
      }

      for (const auto& bind : cmd.textures) {
        const auto& tex = _textures.get(bind.tex);
        _state.bind_texture(tex.id, tex.type, bind.index);
      }

      for (const auto& unif : cmd.uniforms) {
        _state.push_uniform(static_cast<uint32>(unif.location), unif.type, unif.data);
      }

      if (index_buffer) {
        const void* offset = reinterpret_cast<const void*>(cmd.offset*sizeof(uint32));
        const GLenum format = GL_UNSIGNED_INT;
        if (cmd.instances) {
          GL_CALL(glDrawElementsInstanced,
            prog.primitive, cmd.count, format, offset, cmd.instances
          );
        } else {
          GL_CALL(glDrawElements,
            prog.primitive, cmd.count, format, offset
          );
        }
      } else {
        if (cmd.instances) {
          GL_CALL(glDrawArraysInstanced,
            prog.primitive, cmd.offset, cmd.count, cmd.instances
          );
        } else {
          GL_CALL(glDrawArrays,
            prog.primitive, cmd.offset, cmd.count
          );
        }
      }
    }
  }
}

void gl_context::swap_buffers() {
  SHOGLE_GL_SWAP_BUFFERS(_win);
}

} // namespace ntf
