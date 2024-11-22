#include "./shader.hpp"

namespace ntf {

gl_shader::gl_shader(std::string_view src, shader_category type) {
  _compile(src, type);
}

gl_shader& gl_shader::compile(std::string_view src, shader_category type) & {
  _compile(src, type);
  return *this;
}

gl_shader&& gl_shader::compile(std::string_view src, shader_category type) && {
  _compile(src, type);
  return std::move(*this);
}

void gl_shader::unload() {
  if (!compiled()) {
    return;
  }

  SHOGLE_LOG(verbose, "[ntf::gl_shader] Shader destroyed (id: {})", _id);
  glDeleteShader(_id);

  _reset();
}


void gl_shader::_compile(std::string_view src, shader_category type) {
  NTF_ASSERT(type != shader_category::none);

  const auto gltype = enumtogl(type);
  const char* src_data = src.data();
  int succ;
  char log[512];

  GLuint id = glCreateShader(gltype);
  glShaderSource(_id, 1, &src_data, nullptr);
  glCompileShader(_id);
  glGetShaderiv(_id, GL_COMPILE_STATUS, &succ);
  if (!succ) {
    // TODO: Store the log in the shader object?
    glGetShaderInfoLog(id, 512, nullptr, log);
    SHOGLE_LOG(error, "[ntf::gl_shader] Shader compilation failed (id: {}) -> {}", _id, log);
    glDeleteShader(id);
    return;
  }

  if (_id) {
    SHOGLE_LOG(verbose, "[ntf::gl_shader] Shader overwritten ({} -> {})", _id, id);
    glDeleteShader(_id);
  } else {
    SHOGLE_LOG(verbose, "[ntf::gl_shader] Shader compiled (id: {})", id);
  }

  _id = id;
  _type = type;
}

void gl_shader::_reset() {
  _id = 0;
  _type = shader_category::none;
}


gl_shader::~gl_shader() noexcept { unload(); }

gl_shader::gl_shader(gl_shader&& s) noexcept : 
  _id(std::move(s._id)), _type(std::move(s._type)) { s._reset(); }

auto gl_shader::operator=(gl_shader&& s) noexcept -> gl_shader& {
  unload();

  _id = std::move(s._id);
  _type = std::move(s._type);

  s._reset();

  return *this;
}


void gl_shader_program::unload() {
  if (_id) {
    SHOGLE_LOG(verbose, "[ntf::gl_shader_program] Shader program destroyed (id: {})", _id);
    glDeleteProgram(_id);
    _id = 0;
  }
}

void gl_shader_program::use() const {
  NTF_ASSERT(linked(), "Invalid gl_shader_program");
  glUseProgram(_id);
}

auto gl_shader_program::uniform_location(std::string_view name) const -> uniform_type {
  NTF_ASSERT(linked(), "Invalid gl_shader_program");
  return glGetUniformLocation(_id, name.data());
}

void gl_shader_program::set_uniform(uniform_type location, const int val) const {
  glUniform1i(location, val);
}

void gl_shader_program::set_uniform(uniform_type location, const float val) const {
  glUniform1f(location, val);
}

void gl_shader_program::set_uniform(uniform_type location, const vec2& val) const {
  glUniform2fv(location, 1, glm::value_ptr(val));
}

void gl_shader_program::set_uniform(uniform_type location, const vec3& val) const {
  glUniform3fv(location, 1, glm::value_ptr(val));
}

void gl_shader_program::set_uniform(uniform_type location, const vec4& val) const {
  glUniform4fv(location, 1, glm::value_ptr(val));
}

void gl_shader_program::set_uniform(uniform_type location, const mat3& val) const {
  glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(val));
}

void gl_shader_program::set_uniform(uniform_type location, const mat4& val) const {
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(val));
}


void gl_shader_program::_link(std::string_view vert_src, std::string_view frag_src) {
  _link(
    gl_shader{}.compile(vert_src, shader_category::vertex),
    gl_shader{}.compile(frag_src, shader_category::fragment)
  );
}

void gl_shader_program::_link(std::string_view vert_src, std::string_view frag_src,
                             std::string_view geom_src) {
  _link(
    gl_shader{}.compile(vert_src, shader_category::vertex),
    gl_shader{}.compile(frag_src, shader_category::fragment),
    gl_shader{}.compile(geom_src, shader_category::geometry)
  );
}

void gl_shader_program::_reset() {
  _id = 0;
}


gl_shader_program::~gl_shader_program() noexcept { unload(); }

gl_shader_program::gl_shader_program(gl_shader_program&& p) noexcept : 
  _id(std::move(p._id)) { p._reset(); }

auto gl_shader_program::operator=(gl_shader_program&& p) noexcept -> gl_shader_program& {
  unload();

  _id = std::move(p._id);

  p._reset();

  return *this;
}

} // namespace ntf
