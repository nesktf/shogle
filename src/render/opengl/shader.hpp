#pragma once

#include "./opengl.hpp"

namespace ntf {

class gl_pipeline;

class gl_shader {
private:
  gl_shader(gl_context& ctx) :
    _ctx(ctx) {}

private:
  void load(r_shader_type type, std::string_view src);
  void unload();

private:
  gl_context& _ctx;

  GLuint _id{0};
  r_shader_type _type{r_shader_type::none};

public:
  NTF_DISABLE_MOVE_COPY(gl_shader);

private:
  friend class gl_context;
  friend class gl_pipeline;
};

constexpr inline GLenum gl_shader_type_cast(r_shader_type type) {
  switch (type) {
    case r_shader_type::vertex:               return GL_VERTEX_SHADER;
    case r_shader_type::fragment:             return GL_FRAGMENT_SHADER;
    case r_shader_type::geometry:             return GL_GEOMETRY_SHADER;
    case r_shader_type::tesselation_eval:     return GL_TESS_EVALUATION_SHADER;
    case r_shader_type::tesselation_control:  return GL_TESS_CONTROL_SHADER;

    case r_shader_type::compute:              [[fallthrough]];
    case r_shader_type::none:                 return 0;
  }
  return 0;
}

} // namespace ntf
