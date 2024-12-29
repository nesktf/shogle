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
  bool complete() const { return _id != 0; }

private:
  gl_context& _ctx;

  GLuint _id{0};
  r_shader_type _type{r_shader_type::none};

private:
  friend class gl_context;
  friend class gl_pipeline;
};

constexpr inline GLenum gl_shader_type_cast(r_shader_type type) {

}

} // namespace ntf
