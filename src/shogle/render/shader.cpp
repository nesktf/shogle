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


shader::shader(GLuint id, shader_type type) : _id(id), _type(type) {}

shader::shader(std::string_view src, shader_type type) : _type(type) {
  const auto gltype = __enumtogl(type);
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
    throw ntf::error{"[shogle::shader] Shader compilation falied: {}", log};
  }
  log::verbose("[shogle::shader] Shader compiled (id: {})", _id);
}

shader::shader(shader&& s) noexcept : _id(std::move(s._id)), _type(std::move(s._type)) {
  s._id = 0;
}

auto shader::operator=(shader&& s) noexcept -> shader& {
  if (compiled()) {
    unload_shader();
  }
  _id = std::move(s._id);
  _type = std::move(s._type);

  s._id = 0;

  return *this;
}

shader::~shader() {
  if (compiled()) {
    unload_shader();
  }
}

void shader::unload_shader() {
  log::verbose("[shogle::shader] Shader unloaded (id: {})", _id);
  glDeleteShader(_id);
}


shader_program::shader_program(GLuint id) : _id(id) {}

shader_program::shader_program(shader vert, shader frag) {
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
    throw ntf::error{"[shogle::shader_program] Shader program linking failed: {}", log};
  }

  log::verbose("[shogle::shader_program] Shader program linked (id: {})", _id);
}

shader_program::shader_program(shader vert, shader frag, shader geom) {
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
    throw ntf::error{"[shogle::shader_program] Shader program linking failed: {}", log};
  }

  log::verbose("[shogle::shader_program] Shader program linked (id: {})", _id);
}

shader_program::shader_program(shader_program&& p) noexcept :
  _id(std::move(p._id)) { 
  p._id = 0;
}

shader_program& shader_program::operator=(shader_program&& p) noexcept {
  if (_id) {
    unload_program();
  }

  _id = std::move(p._id);

  p._id = 0;

  return *this;
}

shader_program::~shader_program() {
  if (_id) {
    unload_program();
  }
}

shader_uniform shader_program::uniform_location(std::string_view name) const {
  assert(linked() && "Shader program not linked");
  const auto loc = glGetUniformLocation(_id, name.data());
  assert(loc != -1 && "Invald uniform name");
  return loc;
}

void shader_program::unload_program() {
  log::verbose("[shogle::shader_program] Shader program unloaded (id: {})", _id);
  glDeleteProgram(_id);
}


void render_use_shader(const shader_program& shader) {
  assert(shader.linked() && "Shader program not linked");
  glUseProgram(shader._id);
}

void render_set_uniform(shader_uniform location, const int val) {
  glUniform1i(location, val);
}

void render_set_uniform(shader_uniform location, const float val) {
  glUniform1f(location, val);
}

void render_set_uniform(shader_uniform location, const vec2& val) {
  glUniform2fv(location, 1, glm::value_ptr(val));
}

void render_set_uniform(shader_uniform location, const vec3& val) {
  glUniform3fv(location, 1, glm::value_ptr(val));
}

void render_set_uniform(shader_uniform location, const vec4& val) {
  glUniform4fv(location, 1, glm::value_ptr(val));
}

void render_set_uniform(shader_uniform location, const mat3& val) {
  glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(val));
}

void render_set_uniform(shader_uniform location, const mat4& val) {
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(val));
}

} // namespace ntf::shogle
