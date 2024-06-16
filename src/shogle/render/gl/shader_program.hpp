#pragma once

#include <shogle/render/gl/gl.hpp>

namespace ntf::shogle::gl {

template<typename T, typename... TRes>
concept same_as_any = (... or std::same_as<T, TRes>);

template<typename T>
concept vertex_type = (same_as_any<T, vec2, vec3, vec4>);

template<unsigned int _index, typename T>
requires(vertex_type<T>)
struct shader_attribute {
  static constexpr unsigned int index = _index;
  static constexpr size_t stride = sizeof(T);
};

class shader_program;

class shader {
private:
  friend shader_program;

public:
  enum class type {
    vertex,
    geometry,
    fragment
  };

public:
  shader(std::string src, type type);

public:
  void compile();

public:
  bool compiled() { return _shad_id != 0; }

public:
  ~shader();
  shader(shader&&) noexcept;
  shader(const shader&) = delete;
  shader& operator=(shader&&) noexcept;
  shader& operator=(const shader&) = delete;

private:
  std::string _src;
  GLuint _shad_id {0};
  type _type;
};

class mesh;

class shader_program {
protected:
  using uniform_id = int;

protected:
  shader_program() = default;

protected:
  void attach_shaders(shader vertex, shader fragment, shader geometry);
  void attach_shaders(shader vertex, shader fragment);

protected:
  template<typename T>
  void set_uniform(uniform_id location, T val);

  uniform_id uniform_location(const char* name);

public:
  void draw(const mesh& mesh);

public:
  ~shader_program();
  shader_program(shader_program&&) noexcept;
  shader_program(const shader_program&) = delete;
  shader_program& operator=(shader_program&&) noexcept;
  shader_program& operator=(const shader_program&) = delete;

private:
  GLuint _prog_id {0};
};

template<>
void shader_program::set_uniform(uniform_id, int);

template<>
void shader_program::set_uniform(uniform_id, long unsigned int);

template<>
void shader_program::set_uniform(uniform_id, float);

template<>
void shader_program::set_uniform(uniform_id, vec2);

template<>
void shader_program::set_uniform(uniform_id, vec3);

template<>
void shader_program::set_uniform(uniform_id, vec4);

template<>
void shader_program::set_uniform(uniform_id, mat3);

template<>
void shader_program::set_uniform(uniform_id, mat4);

} // namespace ntf::shogle::gl
