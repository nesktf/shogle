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
  template<typename T, typename... S>
  void link(const T& shader, const S&... shaders) {
    assert(shader.compiled() && "Attached shader not compiled");
    glAttachShader(_prog_id, shader.id());
    _last_shader = shader.id();
    link(shaders...);
  } 

protected:
  void link();

public:
  uniform_id uniform_location(const char* name);

  void set_uniform(uniform_id location, int val);
  void set_uniform(uniform_id location, float val);
  void set_uniform(uniform_id location, vec2 val);
  void set_uniform(uniform_id location, vec3 val);
  void set_uniform(uniform_id location, vec4 val);
  void set_uniform(uniform_id location, mat3 val);
  void set_uniform(uniform_id location, mat4 val);

public:
  bool linked() const { return _prog_id != 0; }

public:
  ~shader_program();
  shader_program(shader_program&&) noexcept;
  shader_program(const shader_program&) = delete;
  shader_program& operator=(shader_program&&) noexcept;
  shader_program& operator=(const shader_program&) = delete;

private:
  GLuint _prog_id {0};
  GLint _last_shader {0};
};

} // namespace ntf::shogle
