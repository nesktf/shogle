#pragma once

#include <shogle/render/render.hpp>

namespace ntf::shogle {

using shader_uniform = GLint;

enum class shader_type {
  vertex = 0,
  fragment,
  geometry
};

class shader {
public:

public:
  shader() = default;
  shader(GLuint id, shader_type type);
  shader(std::string_view src, shader_type type);

public:
  shader_type type() const { return _type; }
  bool compiled() const { return _id != 0; }
  GLuint& id() { return _id; } // Not const;

public:
  ~shader();
  shader(shader&&) noexcept;
  shader(const shader&) = delete;
  shader& operator=(shader&&) noexcept;
  shader& operator=(const shader&) = delete;

private:
  void unload_shader();

private:
  GLuint _id{};
  shader_type _type;
};


class shader_program {
public:
  shader_program() = default;
  shader_program(GLuint id);
  shader_program(shader vert, shader frag);
  shader_program(shader vert, shader frag, shader geom);

public:
  shader_uniform uniform_location(std::string_view name) const;
  bool linked() const { return _id != 0; }
  GLuint& id() { return _id; } // Not const

public:
  ~shader_program();
  shader_program(shader_program&&) noexcept;
  shader_program(const shader_program&) = delete;
  shader_program& operator=(shader_program&&) noexcept;
  shader_program& operator=(const shader_program&) = delete;

private:
  void unload_program();

private:
  GLuint _id{};

private:
  friend void render_use_shader(const shader_program& shader);
};

void render_use_shader(const shader_program& shader);

void render_set_uniform(shader_uniform location, const int val);
void render_set_uniform(shader_uniform location, const float val);
void render_set_uniform(shader_uniform location, const vec2& val);
void render_set_uniform(shader_uniform location, const vec3& val);
void render_set_uniform(shader_uniform location, const vec4& val);
void render_set_uniform(shader_uniform location, const mat3& val);
void render_set_uniform(shader_uniform location, const mat4& val);

template<typename T>
void render_set_uniform(const shader_program& shader, std::string_view name, const T& val) {
  const auto location = shader.uniform_location(name);
  render_set_uniform(location, val);
}

inline shader_program load_shader_program(std::string_view vert, std::string_view frag) {
  return shader_program{
    shader{vert, shader_type::vertex},
    shader{frag, shader_type::fragment},
  };
}

inline shader_program load_shader_program(std::string_view vert, std::string_view frag, std::string_view geom) {
  return shader_program{
    shader{vert, shader_type::vertex},
    shader{frag, shader_type::fragment},
    shader{geom, shader_type::geometry}
  };
}

} // namespace ntf::shogle
