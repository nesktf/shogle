#include <shogle/render/shader.hpp>
#include <shogle/render/mesh.hpp>

#include <shogle/core/log.hpp>
#include <shogle/core/error.hpp>

#include <glm/gtc/type_ptr.hpp>

namespace ntf::shogle {

constexpr GLint __enumtogl(shader_type type) {
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

void shader::compile() {
  int succ;
  char log[512];

  const char* _char_src = _src.data();
  _shad_id = glCreateShader(_type);
  glShaderSource(_shad_id, 1, &_char_src, nullptr);
  glCompileShader(_shad_id);
  glGetShaderiv(_shad_id, GL_COMPILE_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(_shad_id, 512, nullptr, log);
    glDeleteShader(_shad_id);
    throw ntf::error{"[shogle::shader] Shader compilation falied: {}", log};
  }
  log::verbose("[shogle::shader] Shader compiled (id: {})", _shad_id);
}

shader::shader(std::string_view src, shader_type type) :
  _src(std::move(src)), _type(__enumtogl(type)) {}

shader::shader(shader&& s) noexcept :
  _src(std::move(s._src)),
  _type(std::move(s._type)),
  _shad_id(std::move(s._shad_id)) {
  s._shad_id = 0; 
}

auto shader::operator=(shader&& s) noexcept -> shader& {
  auto id = _shad_id;
  if (_shad_id) {
    glDeleteShader(_shad_id);
  }

  _src = std::move(s._src);
  _shad_id = s._shad_id;
  _type = s._type;

  s._shad_id = 0;

  log::verbose("[shogle::shader] Shader overwritten (id: {})", id);
  return *this;
}

shader::~shader() {
  if (!_shad_id) return;
  auto id = _shad_id;
  glDeleteShader(_shad_id);
  log::verbose("[shogle::shader] Shader unloaded (id: {})", id);
}

shader_program::shader_program() :
  _prog_id(glCreateProgram()) {}

void shader_program::link() {
  int succ;
  char log[512];

  glLinkProgram(_prog_id);
  glGetProgramiv(_prog_id, GL_LINK_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(_last_shader, 512, nullptr, log);
    throw ntf::error{"[shogle::shader_program] Shader program linking failed: {}", log};
  }

  log::verbose("[shogle::shader_program] Shader program linked (id: {})", _prog_id);
}

auto shader_program::uniform_location(const char* name) -> uniform_id {
  return glGetUniformLocation(_prog_id, name);
}

void shader_program::set_uniform(uniform_id location, const int val) {
  glUseProgram(_prog_id);
  glUniform1i(location, val);
}

void shader_program::set_uniform(uniform_id location, const float val) {
  glUseProgram(_prog_id);
  glUniform1f(location, val);
}

void shader_program::set_uniform(uniform_id location, const vec2& val) {
  glUseProgram(_prog_id);
  glUniform2fv(location, 1, glm::value_ptr(val));
}

void shader_program::set_uniform(uniform_id location, const vec3& val) {
  glUseProgram(_prog_id);
  glUniform3fv(location, 1, glm::value_ptr(val));
}

void shader_program::set_uniform(uniform_id location, const vec4& val) {
  glUseProgram(_prog_id);
  glUniform4fv(location, 1, glm::value_ptr(val));
}

void shader_program::set_uniform(uniform_id location, const mat3& val) {
  glUseProgram(_prog_id);
  glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(val));
}

void shader_program::set_uniform(uniform_id location, const mat4& val) {
  glUseProgram(_prog_id);
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(val));
}

shader_program::shader_program(shader_program&& sh) noexcept :
  _prog_id(std::move(sh._prog_id)) { sh._prog_id = 0; }

shader_program& shader_program::operator=(shader_program&& sh) noexcept {
  auto id = _prog_id;
  glDeleteProgram(_prog_id);

  _prog_id = sh._prog_id;

  sh._prog_id = 0;

  log::verbose("[shogle::shader_program] Shader program overwritten (id: {})", id);
  return *this;
}

shader_program::~shader_program() {
  if (!_prog_id) return;
  auto id = _prog_id;
  glDeleteProgram(_prog_id);
  log::verbose("[shogle::shader_program] Shader program unloaded (id: {})", id);
}

} // namespace ntf::shogle
