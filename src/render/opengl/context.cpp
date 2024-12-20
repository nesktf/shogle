#include "./context.hpp"
#include "../meshes.hpp"

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

  SHOGLE_LOG(debug, "[ntf::gl_context][{}][{}][{}][{}] {}",
             severity_msg, type_msg, src_msg, id, msg);
}

void gl_context::init(GLADloadproc proc) {
  if (!gladLoadGLLoader(proc)) {
    return;
  }

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(gl_context::_debug_callback, this);
  glGenVertexArrays(1, &_state.vao);
  glBindVertexArray(_state.vao);

  glEnable(GL_DEPTH_TEST); // ?

  _proc_fun = proc;
}

void gl_context::destroy() {
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &_state.vao);
  _state.vao = 0;

  glUseProgram(0);
  _state.program = 0;
}

void gl_context::enqueue(r_draw_cmd cmd) {
  _cmds.emplace(cmd);
}

void gl_context::draw_frame() {
  gl_clear_bits(_state.clears, _state.clear_color);

  while (!_cmds.empty()) {
    r_draw_cmd cmd = _cmds.front();
    _cmds.pop();

    auto& pipeline = _pipelines[cmd.pipeline];
    if (_state.program != pipeline._program_id) {
      glUseProgram(pipeline._program_id);
      _state.program = pipeline._program_id;
    }

    auto& vbo = _buffers[cmd.vertex_buffer];
    if (_state.vbo != vbo._id) {
      glBindBuffer(GL_ARRAY_BUFFER, vbo._id);

      // HAS to be reconfigured each time the vertex buffer is rebound
      const auto& [attribs, stride] = _attribs[pipeline._attrib_handle];
      for (const auto& attrib : attribs) {
        const uint32 type_dim = r_attrib_type_dim(attrib.type);
        NTF_ASSERT(type_dim);

        const GLenum gl_underlying_type = gl_attrib_type_underlying_cast(attrib.type);
        NTF_ASSERT(gl_underlying_type);

        glVertexAttribPointer(
          attrib.location,
          type_dim,
          gl_underlying_type,
          GL_FALSE, // Don't normalize,
          stride,
          reinterpret_cast<void*>(attrib.offset)
        );
        glEnableVertexAttribArray(attrib.location);
      }

      _state.vbo = vbo._id;
    }

    if (cmd.textures) {
      _state.enabled_tex = 0;
      for (uint32 i = 0; i < cmd.texture_count; ++i) {
        auto& tex = _textures[cmd.textures[i]];
        const uint32 gltype = gl_texture_type_cast(tex.type(), tex.is_array());
        NTF_ASSERT(gltype);

        glActiveTexture(GL_TEXTURE0+i);
        glBindTexture(gltype, tex._id);
        _state.enabled_tex |= 1 << i;
      }
    }

    if (cmd.uniforms) {
      for (uint32 i = 0; i < cmd.uniform_count; ++i) {
        auto& unif = cmd.uniforms[i];
        gl_pipeline::push_uniform(unif.location, unif.type, unif.data);
      }
    }

    const uint32 glprim = gl_primitive_cast(cmd.primitive);
    if (cmd.index_buffer == r_resource_tombstone) {
      glDrawArrays(glprim, cmd.draw_offset, cmd.draw_count);
      continue;
    }

    auto& ebo = _buffers[cmd.index_buffer];
    if (_state.ebo != ebo._id) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo._id);
      _state.ebo = ebo._id;
    }
    glDrawElements(glprim, cmd.draw_count, GL_UNSIGNED_INT,
                   reinterpret_cast<void*>(cmd.draw_offset*sizeof(uint32)));
  }
}

void gl_context::viewport(uint32 x, uint32 y, uint32 w, uint32 h) {
  glViewport(x, y, w, h);
  _state.viewport.x = x;
  _state.viewport.y = y;
  _state.viewport.z = w;
  _state.viewport.w = h;
}

void gl_context::viewport(uint32 w, uint32 h) {
  glViewport(_state.viewport.x, _state.viewport.y, w, h);
  _state.viewport.z = w,
  _state.viewport.w = h;
}

void gl_context::toggle_clear(r_clear clear) {
  _state.clears ^= clear;
}

void gl_context::clear_color(color4 color) {
  _state.clear_color = color;
}

std::string_view gl_context::name() const {
  return reinterpret_cast<const char*>(glGetString(GL_RENDERER));
}

std::string_view gl_context::vendor() const {
  return reinterpret_cast<const char*>(glGetString(GL_VENDOR));
}

std::string_view gl_context::version() const {
  return reinterpret_cast<const char*>(glGetString(GL_VERSION));
}

auto gl_context::create_texture(r_texture_info info) -> expected<texture_handle, r_texture_err> {
}

auto gl_context::create_buffer(r_buffer_info info) -> expected<buffer_handle, r_buffer_err> {

}

auto gl_context::create_pipeline(r_pipeline_info info) -> expected<pipeline_handle, r_pipeline_err> {

}

auto gl_context::create_shader(r_shader_info info) -> expected<shader_handle, r_shader_err> {

}
//
// gl_context& gl_context::current_context() {
//   NTF_ASSERT(_current_context, "Current gl_context is invalid");
//   return *_current_context;
// }
//
// void gl_context::set_current_context(gl_context& ctx) {
//   _current_context = &ctx;
// }
//
// void gl_context::destroy() {
//   NTF_ASSERT(valid());
//   if (this == _current_context) {
//     _current_context = nullptr;
//   }
// }
//
// void gl_context::set_state.viewport(std::size_t w, std::size_t h) const {
//   NTF_ASSERT(valid());
//   glViewport(0, 0, w, h);
// }
//
// void gl_context::set_state.viewport(ivec2 sz) const {
//   NTF_ASSERT(valid());
//   glViewport(0, 0, sz.x, sz.y);
// }
//
// void gl_context::set_state.viewport(std::size_t x, std::size_t y, std::size_t w, std::size_t h) const {
//   NTF_ASSERT(valid());
//   glViewport(x, y, w, h);
// }
//
// void gl_context::set_state.viewport(ivec2 pos, ivec2 sz) const {
//   NTF_ASSERT(valid());
//   glViewport(pos.x, pos.y, sz.x, sz.y);
// }
//
// void gl_context::clear_state.viewport(color4 color, clear flag) const {
//   NTF_ASSERT(valid());
//   GLbitfield mask = GL_COLOR_BUFFER_BIT;
//   if ((flag & clear::depth) != clear::none) {
//     mask |= GL_DEPTH_BUFFER_BIT;
//   }
//   if ((flag & clear::stencil) != clear::none) {
//     mask |= GL_STENCIL_BUFFER_BIT;
//   }
//   glClearColor(color.r, color.g, color.b, color.a);
//   glClear(mask);
// }
//
// void gl_context::clear_state.viewport(color3 color, clear flag) const {
//   clear_state.viewport(color4{color, 1.0f}, flag);
// }
//
// void gl_context::set_stencil_test(bool flag) const {
//   NTF_ASSERT(valid());
//   if (flag) {
//     glEnable(GL_STENCIL_TEST);
//   } else {
//     glDisable(GL_STENCIL_TEST);
//   }
// }
//
// void gl_context::set_depth_test(bool flag) const {
//   NTF_ASSERT(valid());
//   if (flag) {
//     glEnable(GL_DEPTH_TEST);
//   } else {
//     glDisable(GL_DEPTH_TEST);
//   }
// }
//
// void gl_context::set_blending(bool flag) const {
//   NTF_ASSERT(valid());
//   if (flag) {
//     glEnable(GL_BLEND);
//     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //TODO: set_blending_fun
//   } else {
//     glDisable(GL_BLEND);
//   }
// }
//
// void gl_context::set_depth_fun(depth_fun fun) const {
//   NTF_ASSERT(valid());
//   //TODO: More funcs
//   switch (fun) {
//     case depth_fun::less: {
//       glDepthFunc(GL_LESS);
//       break;
//     }
//     case depth_fun::lequal: {
//       glDepthFunc(GL_LEQUAL);
//       break;
//     }
//   }
// }
//
// auto gl_context::make_quad(mesh_buffer vert_buff, mesh_buffer ind_buff) const -> mesh {
//   NTF_ASSERT(valid());
//   auto quad = mesh{}
//     .vertices(&ntf::quad_vertices[0], sizeof(ntf::quad_vertices), vert_buff)
//     .indices(&ntf::quad_indices[0], sizeof(ntf::quad_indices), ind_buff)
//     .attributes(
//       shader_attribute<0, vec3>{}, shader_attribute<1, vec3>{}, shader_attribute<2, vec2>{}
//     );
//   return quad;
// }
//
// auto gl_context::make_cube(mesh_buffer vert_buff, mesh_buffer) const -> mesh {
//   NTF_ASSERT(valid());
//   auto cube = mesh{}
//     .vertices(&ntf::cube_vertices[0], sizeof(ntf::cube_vertices), vert_buff)
//     .attributes(
//       shader_attribute<0, vec3>{}, shader_attribute<1, vec3>{}, shader_attribute<2, vec2>{}
//     );
//   return cube;
// }
//
// void gl_context::draw(mesh_primitive prim, const mesh& mesh, std::size_t offset, uint count) const{
//   NTF_ASSERT(valid());
//
//   if (!mesh.valid()) {
//     SHOGLE_LOG(warning, "[ntf::gl_context::draw_instanced] Attempted to draw empty mesh");
//     return;
//   }
//
//   const auto glprim = enumtogl(prim);
//   glBindVertexArray(mesh.vao());
//   if (mesh.has_indices()) {
//     // indices in glDrawElements is an offset in the current EBO
//     // it advances offset*sizeof(unsigned int) in the buffer
//     glDrawElements(glprim, count > 0 ? count : mesh.elem_count(), GL_UNSIGNED_INT,
//                    reinterpret_cast<void*>(offset));
//   } else {
//     glDrawArrays(glprim, offset, count > 0 ? count : mesh.elem_count());
//   }
//   glBindVertexArray(0);
// }
//
// void gl_context::draw_instanced(mesh_primitive prim, const mesh& mesh, uint primcount,
//                                std::size_t offset, uint count) const {
//   NTF_ASSERT(valid());
//
//   if (!mesh.valid()) {
//     SHOGLE_LOG(warning, "[ntf::gl_context::draw_instanced] Attempted to draw empty mesh");
//     return;
//   }
//
//   const auto glprim = enumtogl(prim);
//   glBindVertexArray(mesh.vao());
//   if (mesh.has_indices()) {
//     // indices in glDrawElementsInstanced is also an offset in the current EBO
//     // it advances offset*sizeof(unsigned int) in the buffer
//     glDrawElementsInstanced(glprim, count > 0 ? count : mesh.elem_count(), GL_UNSIGNED_INT,
//                             reinterpret_cast<void*>(offset), primcount);
//   } else {
//     glDrawArraysInstanced(glprim, offset, count > 0 ? count : mesh.elem_count(), primcount);
//   }
//   glBindVertexArray(0);
// }
//
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
//
// void gl_context::draw_text(const font& font, std::string_view text) const {
//   draw_text(font, vec2{0.f}, 0.f, text);
// }
//
} // namespace ntf
