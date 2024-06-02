#include <shogle/render/gl/shader_program.hpp>

#include <shogle/render/gl/mesh.hpp>

#include <shogle/core/log.hpp>
#include <shogle/core/error.hpp>

#include <glm/gtc/type_ptr.hpp>

namespace ntf::shogle::gl {

void shader_program::attach_shaders(shader vertex, shader fragment) {
  assert(vertex.compiled() && fragment.compiled());

  int succ;
  char log[512];

  _prog_id = glCreateProgram();
  glAttachShader(_prog_id, vertex._shad_id);
  glAttachShader(_prog_id, fragment._shad_id);
  glLinkProgram(_prog_id);
  glGetProgramiv(_prog_id, GL_LINK_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(vertex._shad_id, 512, nullptr, log);
    throw ntf::error{"[gl::shader_program::attach_shaders] Shader linking failed: {}", log};
  }

  log::verbose("[gl::shader_program::attach_shaders] Shader program created (id: {})", _prog_id);
}

void shader_program::attach_shaders(shader vertex, shader fragment, shader geometry) {
  assert(vertex.compiled() && fragment.compiled() && geometry.compiled());

  int succ;
  char log[512];

  _prog_id = glCreateProgram();
  glAttachShader(_prog_id, vertex._shad_id);
  glAttachShader(_prog_id, geometry._shad_id);
  glAttachShader(_prog_id, fragment._shad_id);
  glLinkProgram(_prog_id);
  glGetProgramiv(_prog_id, GL_LINK_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(vertex._shad_id, 512, nullptr, log);
    throw ntf::error{"[gl::shader_program::attach_shaders] Shader linking failed: {}", log};
  }

  log::verbose("[gl::shader_program::attach_shaders] Shader program created (id: {})", _prog_id);
}

shader_program::shader_program(shader_program&& sh) noexcept :
  _prog_id(sh._prog_id) { sh._prog_id = 0; }

shader_program& shader_program::operator=(shader_program&& sh) noexcept {
  auto id = _prog_id;
  glDeleteProgram(_prog_id);

  _prog_id = sh._prog_id;

  sh._prog_id = 0;

  log::verbose("[gl::shader_program] Shader program overwritten (id: {})", id);
  return *this;
}

shader_program::~shader_program() {
  if (!_prog_id) return;
  auto id = _prog_id;
  glDeleteProgram(_prog_id);
  log::verbose("[gl::shader_program] Shader program destroyed (id: {})", id);
}

shader_program::uniform_id shader_program::uniform_location(const char* name) {
  return glGetUniformLocation(_prog_id, name);
}

template<>
void shader_program::set_uniform(uniform_id location, int val) {
  glUseProgram(_prog_id);
  glUniform1i(location, val);
}

template<>
void shader_program::set_uniform(uniform_id location, long unsigned int val) {
  set_uniform<int>(location, val);
}

template<>
void shader_program::set_uniform(uniform_id location, float val) {
  glUseProgram(_prog_id);
  glUniform1f(location, val);
}

template<>
void shader_program::set_uniform(uniform_id location, vec2 val) {
  glUseProgram(_prog_id);
  glUniform2fv(location, 1, glm::value_ptr(val));
}

template<>
void shader_program::set_uniform(uniform_id location, vec3 val) {
  glUseProgram(_prog_id);
  glUniform3fv(location, 1, glm::value_ptr(val));
}

template<>
void shader_program::set_uniform(uniform_id location, vec4 val) {
  glUseProgram(_prog_id);
  glUniform4fv(location, 1, glm::value_ptr(val));
}

template<>
void shader_program::set_uniform(uniform_id location, mat3 val) {
  glUseProgram(_prog_id);
  glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(val));
}

template<>
void shader_program::set_uniform(uniform_id location, mat4 val) {
  glUseProgram(_prog_id);
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(val));
}

void shader_program::draw(const mesh& mesh) {
  mesh.draw();
}

} // namespace ntf::shogle::gl
