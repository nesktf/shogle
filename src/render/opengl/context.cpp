#include "./context.hpp"

namespace ntf {

void gl_context::_debug_callback(GLenum src, GLenum type, GLuint id, GLenum severity,
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
    SHOGLE_LOG(error, "[ntf::gl_context][{}][{}][{}][{}] {}",
               severity_msg, type_msg, src_msg, id, msg);
  } else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
    SHOGLE_LOG(verbose, "[ntf::gl_context][{}][{}][{}][{}] {}",
               severity_msg, type_msg, src_msg, id, msg);
  } else {
    SHOGLE_LOG(debug, "[ntf::gl_context][{}][{}][{}][{}] {}",
               severity_msg, type_msg, src_msg, id, msg);
  }
}

bool gl_context::_init_state() {
#ifdef SHOGLE_ENABLE_IMGUI
  ImGui_ImplOpenGL3_Init("#version 130");
#endif

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(gl_context::_debug_callback, this);
  glGenVertexArrays(1, &_glstate.vao);
  glBindVertexArray(_glstate.vao);

  glEnable(GL_DEPTH_TEST); // ?
  return true;
}

void gl_context::destroy() {
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &_glstate.vao);
  _glstate.vao = 0;

  glUseProgram(0);
  _glstate.program = 0;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  _glstate.vbo = 0;

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  _glstate.ebo = 0;

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  _glstate.fbo = 0;

  _glstate.viewport = uvec4{0, 0, 0, 0};
  _glstate.clear_flags = r_clear::none;
  _glstate.clear_color = color4{.3f, .3f, .3f, 1.f};

  _textures.clear();
  _buffers.clear();
  _pipelines.clear();
  _shaders.clear();
  _framebuffers.clear();
  _cmds = {};

  _proc_fun = nullptr;
  _swap_buffers = {};

#ifdef SHOGLE_ENABLE_IMGUI
  ImGui_ImplOpenGL3_Shutdown();
#endif
}

void gl_context::enqueue(r_draw_cmd cmd) {
  _cmds.emplace(cmd);
}

void gl_context::start_frame() {
#ifdef SHOGLE_ENABLE_IMGUI
  ImGui_ImplOpenGL3_NewFrame();
  ImGui::NewFrame();
#endif
}

void gl_context::end_frame() {
  gl_clear_bits(_glstate.clear_flags, _glstate.clear_color);

  auto draw_things = [](GLenum prim, uint32 offset, uint32 count, uint32 instances, bool indices) {
    if (!indices) {
      if (instances > 1) {
        glDrawArraysInstanced(prim, offset, count, instances);
      } else {
        glDrawArrays(prim, offset, count);
      }
    } else {
      if (instances > 1) {
        glDrawElementsInstanced(prim, count, GL_UNSIGNED_INT,
                                reinterpret_cast<void*>(offset*sizeof(uint32)), instances);
      } else {
        glDrawElements(prim, count, GL_UNSIGNED_INT,
                       reinterpret_cast<void*>(offset*sizeof(uint32)));
      }
    }
  };

  while (!_cmds.empty()) {
    r_draw_cmd cmd = _cmds.front();
    _cmds.pop();

    uint32 fbo = 0;
    uvec4 vp = viewport();

    if (cmd.framebuffer) {
      NTF_ASSERT(cmd.framebuffer.api == RENDER_API);
      auto& fb = resource<gl_framebuffer>({}, cmd.framebuffer.handle);
      fbo = fb._fbo;
      vp = fb.viewport();
    }

    if (_glstate.fbo != fbo) {
      glBindFramebuffer(GL_FRAMEBUFFER, fbo);
      glViewport(vp.x, vp.y, vp.z, vp.w);
      _glstate.fbo = fbo;
    }

    bool rebind_attributes = false;

    NTF_ASSERT(cmd.pipeline && cmd.pipeline.api == RENDER_API);
    auto& pipeline = resource<gl_pipeline>({}, cmd.pipeline.handle);
    if (_glstate.program != pipeline._program_id) {
      glUseProgram(pipeline._program_id);
      _glstate.program = pipeline._program_id;
      rebind_attributes = true;
    }

    NTF_ASSERT(cmd.vertex_buffer && cmd.vertex_buffer.api == RENDER_API);
    auto& vbo = resource<gl_buffer>({}, cmd.vertex_buffer.handle);
    if (_glstate.vbo != vbo._id) {
      glBindBuffer(GL_ARRAY_BUFFER, vbo._id);
      _glstate.vbo = vbo._id;
      rebind_attributes = true; // HAS to be reconfigured each time the vertex buffer is rebound
    }

    bool use_indices = bool{cmd.index_buffer};
    if (use_indices) {
      NTF_ASSERT(cmd.index_buffer.api == RENDER_API);
      auto& ebo = resource<gl_buffer>({}, cmd.index_buffer.handle);
      if (_glstate.ebo != ebo._id) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo._id);
        _glstate.ebo = ebo._id;
      }
    }

    if (rebind_attributes) {
      for (const auto& attrib : pipeline._attribs) {
        const uint32 type_dim = r_attrib_type_dim(attrib.type);
        NTF_ASSERT(type_dim);

        const GLenum gl_underlying_type = gl_attrib_underlying_type_cast(attrib.type);
        NTF_ASSERT(gl_underlying_type);

        glVertexAttribPointer(
          attrib.location,
          type_dim,
          gl_underlying_type,
          GL_FALSE, // Don't normalize,
          pipeline._stride,
          reinterpret_cast<void*>(attrib.offset)
        );
        glEnableVertexAttribArray(attrib.location);
      }
    }

    if (cmd.textures) {
      _glstate.enabled_tex = 0;
      for (uint32 i = 0; i < cmd.texture_count; ++i) {
        auto& tex = resource<gl_texture>({}, cmd.textures[i].handle);
        const GLenum gltype = gl_texture_type_cast(tex.type(), tex.is_array());
        NTF_ASSERT(gltype);

        glActiveTexture(GL_TEXTURE0+i);
        glBindTexture(gltype, tex._id);
        _glstate.enabled_tex |= 1 << i;
      }
    }

    if (cmd.uniforms) {
      for (uint32 i = 0; i < cmd.uniform_count; ++i) {
        auto& unif = cmd.uniforms[i];
        gl_pipeline::push_uniform(unif.location, unif.type, unif.data);
      }
    }

    const GLenum glprim = gl_primitive_cast(cmd.primitive);
    NTF_ASSERT(glprim);
    draw_things(glprim, cmd.draw_offset, cmd.draw_count, cmd.instance_count, use_indices);
  }

#ifdef SHOGLE_ENABLE_IMGUI
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif

  _swap_buffers();
}

void gl_context::viewport(uint32 x, uint32 y, uint32 w, uint32 h) {
  // glViewport(x, y, w, h);
  _glstate.viewport.x = x;
  _glstate.viewport.y = y;
  _glstate.viewport.z = w;
  _glstate.viewport.w = h;
}

void gl_context::viewport(uint32 w, uint32 h) {
  viewport(_glstate.viewport.x, _glstate.viewport.y, w, h);
}

void gl_context::viewport(uvec2 pos, uvec2 size) {
  viewport(pos.x, pos.y, size.x, size.y);
}

void gl_context::viewport(uvec2 size) {
  viewport(size.x, size.y);
}

void gl_context::clear_color(float32 r, float32 g, float32 b, float32 a) {
  _glstate.clear_color.r = r;
  _glstate.clear_color.g = g;
  _glstate.clear_color.b = b;
  _glstate.clear_color.a = a;
}

void gl_context::clear_color(float32 r, float32 g, float32 b) {
  clear_color(r, g, b, _glstate.clear_color.a);
}

void gl_context::clear_color(color4 color) {
  clear_color(color.r, color.g, color.b, color. a);
}

void gl_context::clear_color(color3 color) {
  clear_color(color.r, color.g, color.b);
}

void gl_context::clear_flags(r_clear clear) {
  _glstate.clear_flags = clear;
}

std::string_view gl_context::name_str() const {
  return reinterpret_cast<const char*>(glGetString(GL_RENDERER));
}

std::string_view gl_context::vendor_str() const {
  return reinterpret_cast<const char*>(glGetString(GL_VENDOR));
}

std::string_view gl_context::version_str() const {
  return reinterpret_cast<const char*>(glGetString(GL_VERSION));
}

auto gl_context::make_texture(r_texture_descriptor desc)
                                                      -> expected<texture_handle, gl_texture_err> {
  if (!gl_validate_descriptor(desc)) {
    return unexpected<gl_texture_err>{gl_texture_err::none}; // ?
  }

  r_handle_value handle = _textures.acquire(*this);
  auto& tex = _textures.get(handle);

  tex.load(desc.type, desc.format, desc.sampler, desc.addressing, desc.texels,
           desc.mipmap_level, desc.count, desc.extent);
  if (!tex.complete()) {
    _buffers.push(handle);
    return unexpected<gl_texture_err>{gl_texture_err::none}; // ?
  }

  return texture_handle{*this, handle};
}

auto gl_context::make_buffer(r_buffer_descriptor desc) 
                                                        -> expected<buffer_handle, gl_buffer_err> {
  if (!gl_validate_descriptor(desc)) {
    return unexpected<gl_buffer_err>{gl_buffer_err::none};
  }

  r_handle_value handle = _buffers.acquire(*this);
  auto& buff = _buffers.get(handle);

  buff.load(desc.type, desc.data, desc.size);
  if (!buff.complete()) {
    _buffers.push(handle);
    return unexpected<gl_buffer_err>{gl_buffer_err::none};
  }

  return buffer_handle{*this, handle};
}

auto gl_context::make_pipeline(r_pipeline_descriptor desc) 
                                                    -> expected<pipeline_handle, gl_pipeline_err> {
  if (!gl_validate_descriptor(desc)) {
    return unexpected<gl_pipeline_err>{gl_pipeline_err::none};
  }

  r_handle_value handle = _pipelines.acquire(*this);
  auto& pipeline = _pipelines.get(handle);

  std::vector<gl_shader*> stages(desc.stage_count);
  for (uint32 i = 0; i < desc.stage_count; ++i) {
    stages[i] = &_shaders.get(desc.stages[i].handle);
  }

  pipeline.load(stages.data(), desc.stage_count, desc.attribs, desc.attrib_count);
  if (!pipeline.complete()) {
    _pipelines.push(handle);
    return unexpected<gl_pipeline_err>{gl_pipeline_err::none};
  }

  return pipeline_handle{*this, handle};
}

auto gl_context::make_shader(r_shader_descriptor desc)
                                                        -> expected<shader_handle, gl_shader_err> {
  if (!gl_validate_descriptor(desc)) {
    return unexpected<gl_shader_err>{gl_shader_err::none};
  }

  r_handle_value handle = _shaders.acquire(*this);
  auto& shader = _shaders.get(handle);

  shader.load(desc.type, desc.source);
  if (!shader.complete()) {
    _shaders.push(handle);
    return unexpected<gl_shader_err>{gl_shader_err::none};
  }

  return shader_handle{*this, handle};
}

auto gl_context::make_framebuffer(r_framebuffer_descriptor desc) 
                                              -> expected<framebuffer_handle, gl_framebuffer_err> {
  if (!gl_validate_descriptor(desc)) {
    return unexpected<gl_framebuffer_err>{gl_framebuffer_err::none};
  }

  r_handle_value handle = _framebuffers.acquire(*this);
  auto& fbo = _framebuffers.get(handle);

  const uvec4& vp = desc.viewport;
  fbo.load(vp.x, vp.y, vp.z, vp.w, desc.sampler, desc.addressing);
  if (!fbo.complete()) {
    _framebuffers.push(handle);
    return unexpected<gl_framebuffer_err>{gl_framebuffer_err::none};
  }

  return framebuffer_handle{*this, handle};
}


// void gl_context::draw_text(const font& font, vec2 pos, float scale, std::string_view text) const {
//   NTF_ASSERT(valid());
//
//   if (!font.valid() || font.empty()) {
//     SHOGLE_LOG(warning, "[ntf::gl_context::draw_text] Attempted to draw \"{}\" with empty font", 
//                text);
//     return;
//   }
//
//   glBindVertexArray(font.vao());
//
//   float x = pos.x, y = pos.y;
//   const auto& atlas = font.atlas();
//   for (const auto c : text) {
//     GLuint tex;
//     font_glyph glyph;
//     if (atlas.find(c) != atlas.end()) {
//       std::tie(tex, glyph) = atlas.at(c);
//     } else {
//       std::tie(tex, glyph) = atlas.at('?');
//     }
//
//     float xpos = x + glyph.bearing.x*scale;
//     float ypos = y - glyph.bearing.y*scale;
//
//     float w = glyph.size.x*scale;
//     float h = glyph.size.y*scale;
//
//     float vert[6][4] {
//       { xpos,     ypos + h, 0.0f, 1.0f },
//       { xpos,     ypos,     0.0f, 0.0f },
//       { xpos + w, ypos,     1.0f, 0.0f },
//
//       { xpos,     ypos + h, 0.0f, 1.0f },
//       { xpos + w, ypos,     1.0f, 0.0f },
//       { xpos + w, ypos + h, 1.0f, 1.0f }
//     };
//     glBindTexture(GL_TEXTURE_2D, tex);
//
//     glBindBuffer(GL_ARRAY_BUFFER, font.vbo());
//     glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vert), vert);
//
//     glDrawArrays(GL_TRIANGLES, 0, 6);
//
//     x += (glyph.advance >> 6)*scale;
//   }
//   glBindVertexArray(0);
//   glBindTexture(GL_TEXTURE_2D, 0);
// }

} // namespace ntf
