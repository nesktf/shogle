#pragma once

#include <shogle/render/render.hpp>

namespace ntf::shogle {

class mesh;

enum class shader_type {
  vertex = 0,
  fragment,
  geometry,
};


class shader {
public:
  shader(std::string_view src, shader_type type);

public:
  void compile();

public:
  bool compiled() const { return _shad_id != 0; }
  GLuint id() const { return _shad_id; }

public:
  ~shader();
  shader(shader&&) noexcept;
  shader(const shader&) = delete;
  shader& operator=(shader&&) noexcept;
  shader& operator=(const shader&) = delete;

private:
  std::string_view _src;
  GLint _type;
  GLuint _shad_id {0};
};


class shader_program {
public:
  using uniform_id = GLint;

public:
  shader_program();

public:
  void enable();

  void set_uniform(uniform_id location, const int val);
  void set_uniform(uniform_id location, const float val);
  void set_uniform(uniform_id location, const vec2& val);
  void set_uniform(uniform_id location, const vec3& val);
  void set_uniform(uniform_id location, const vec4& val);
  void set_uniform(uniform_id location, const mat3& val);
  void set_uniform(uniform_id location, const mat4& val);

public:
  template<typename T, typename... S>
  void link(const T& shader, const S&... shaders) {
    assert(shader.compiled() && "Attached shader not compiled");
    glAttachShader(_prog_id, shader.id());
    _last_shader = shader.id();
    link(shaders...);
  } 

public:
  uniform_id uniform_location(const char* name);
  bool linked() const { return _prog_id != 0; }

public:
  ~shader_program();
  shader_program(shader_program&&) noexcept;
  shader_program(const shader_program&) = delete;
  shader_program& operator=(shader_program&&) noexcept;
  shader_program& operator=(const shader_program&) = delete;

protected:
  void link();

private:
  GLuint _prog_id {0};
  GLint _last_shader {0};
};

} // namespace ntf::shogle
