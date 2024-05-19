#pragma once

#include <shogle/render/gl/shader.hpp>

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