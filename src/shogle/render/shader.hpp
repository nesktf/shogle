#pragma once

#include <shogle/core/error.hpp>
#include <shogle/core/log.hpp>

#include <shogle/render/render.hpp>

#include <glm/gtc/type_ptr.hpp>

namespace ntf {

using shader_uniform = GLint;

enum class shader_type {
  vertex = 0,
  fragment,
  geometry
};


namespace impl {

constexpr GLint enumtogl(shader_type type) {
  switch (type) {
    case shader_type::vertex:
      return GL_VERTEX_SHADER;
    case shader_type::fragment:
      return GL_FRAGMENT_SHADER;
    case shader_type::geometry:
      return GL_GEOMETRY_SHADER;
  }
  return 0; // shutup gcc
}

} // namespace impl


class shader {
public:
  shader() = default;
  shader(GLuint id, shader_type type) : _id(id), _type(type) {}
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
  shader_program(GLuint id) : _id(id) {}
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


inline shader::shader(std::string_view src, shader_type type) : _type(type) {
  const auto gltype = impl::enumtogl(type);
  int succ;
  char log[512];

  const char* _char_src = src.data();
  _id = glCreateShader(gltype);
  glShaderSource(_id, 1, &_char_src, nullptr);
  glCompileShader(_id);
  glGetShaderiv(_id, GL_COMPILE_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(_id, 512, nullptr, log);
    glDeleteShader(_id);
    throw ntf::error{"[ntf::shader] Shader compilation falied: {}", log};
  }
  log::verbose("[ntf::shader] Shader compiled (id: {})", _id);
}

inline shader::shader(shader&& s) noexcept : _id(std::move(s._id)), _type(std::move(s._type)) { s._id = 0; }

inline auto shader::operator=(shader&& s) noexcept -> shader& {
  if (compiled()) {
    unload_shader();
  }
  _id = std::move(s._id);
  _type = std::move(s._type);

  s._id = 0;

  return *this;
}

inline shader::~shader() {
  if (compiled()) {
    unload_shader();
  }
}

inline void shader::unload_shader() {
  log::verbose("[ntf::shader] Shader unloaded (id: {})", _id);
  glDeleteShader(_id);
}

inline shader_program::shader_program(shader vert, shader frag) {
  assert(vert.compiled() && vert.type() == shader_type::vertex && "Invalid vertex shader");
  assert(frag.compiled() && frag.type() == shader_type::fragment && "Invalid fragment shader");

  int succ;
  char log[512];

  _id = glCreateProgram();
  glAttachShader(_id, vert.id());
  glAttachShader(_id, frag.id());
  glLinkProgram(_id);
  glGetProgramiv(_id, GL_LINK_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(vert.id(), 512, nullptr, log);
    throw ntf::error{"[ntf::shader_program] Shader program linking failed: {}", log};
  }

  log::verbose("[ntf::shader_program] Shader program linked (id: {})", _id);
}

inline shader_program::shader_program(shader vert, shader frag, shader geom) {
  assert(vert.compiled() && vert.type() == shader_type::vertex && "Invalid vertex shader");
  assert(frag.compiled() && frag.type() == shader_type::fragment && "Invalid fragment shader");
  assert(geom.compiled() && geom.type() == shader_type::geometry && "Invalid geometry shader");

  int succ;
  char log[512];

  _id = glCreateProgram();
  glAttachShader(_id, vert.id());
  glAttachShader(_id, frag.id());
  glAttachShader(_id, geom.id());
  glLinkProgram(_id);
  glGetProgramiv(_id, GL_LINK_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(vert.id(), 512, nullptr, log);
    throw ntf::error{"[ntf::shader_program] Shader program linking failed: {}", log};
  }

  log::verbose("[ntf::shader_program] Shader program linked (id: {})", _id);
}

inline shader_program::shader_program(shader_program&& p) noexcept : _id(std::move(p._id)) { p._id = 0; }

inline shader_program& shader_program::operator=(shader_program&& p) noexcept {
  unload_program();

  _id = std::move(p._id);

  p._id = 0;

  return *this;
}

inline shader_program::~shader_program() { unload_program(); }

inline shader_uniform shader_program::uniform_location(std::string_view name) const {
  assert(linked() && "Shader program not linked");
  const auto loc = glGetUniformLocation(_id, name.data());
  assert(loc != -1 && "Invald uniform name");
  return loc;
}

inline void shader_program::unload_program() {
  if (_id) {
    log::verbose("[ntf::shader_program] Shader program unloaded (id: {})", _id);
    glDeleteProgram(_id);
  }
}


inline void render_use_shader(const shader_program& shader) {
  assert(shader.linked() && "Shader program not linked");
  glUseProgram(shader._id);
}

inline void render_set_uniform(shader_uniform location, const int val) {
  glUniform1i(location, val);
}

inline void render_set_uniform(shader_uniform location, const float val) {
  glUniform1f(location, val);
}

inline void render_set_uniform(shader_uniform location, const vec2& val) {
  glUniform2fv(location, 1, glm::value_ptr(val));
}

inline void render_set_uniform(shader_uniform location, const vec3& val) {
  glUniform3fv(location, 1, glm::value_ptr(val));
}

inline void render_set_uniform(shader_uniform location, const vec4& val) {
  glUniform4fv(location, 1, glm::value_ptr(val));
}

inline void render_set_uniform(shader_uniform location, const mat3& val) {
  glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(val));
}

inline void render_set_uniform(shader_uniform location, const mat4& val) {
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(val));
}

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

} // namespace ntf
