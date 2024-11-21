#include "./shader.hpp"

namespace ntf {

void gl_shader::load(std::string_view src, shader_category type) {
  NTF_ASSERT(_id == 0, "gl_shader already initialized");
  _type = type;
  const auto gltype = enumtogl(type);
  int succ;
  char log[512];

  const char* _char_src = src.data();
  _id = glCreateShader(gltype);
  glShaderSource(_id, 1, &_char_src, nullptr);
  glCompileShader(_id);
  glGetShaderiv(_id, GL_COMPILE_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(_id, 512, nullptr, log);
    SHOGLE_LOG(error, "[ntf::gl_shader] Shader compilation failed (id: {}) -> {}", _id, log);
    glDeleteShader(_id);
    _id = 0;
    return;
  }

  SHOGLE_LOG(verbose, "[ntf::gl_shader] Shader compiled (id: {})", _id);
}

void gl_shader::unload() {
  if (compiled()) {
    SHOGLE_LOG(verbose, "[ntf::gl_shader] Shader destroyed (id: {})", _id);
    glDeleteShader(_id);
    _id = 0;
  }
}

gl_shader::~gl_shader() noexcept { unload(); }

gl_shader::gl_shader(gl_shader&& s) noexcept : 
  _id(std::move(s._id)), _type(std::move(s._type)) { s._id = 0; }

auto gl_shader::operator=(gl_shader&& s) noexcept -> gl_shader& {
  unload();

  _id = std::move(s._id);
  _type = std::move(s._type);

  s._id = 0;

  return *this;
}


void gl_shader_program::load(shader_type vert, shader_type frag) {
  NTF_ASSERT(_id != 0, "gl_shader_program already initialized");

  NTF_ASSERT(vert.compiled() && vert.type() == shader_category::vertex,
             "Invalid vertex shader");
  NTF_ASSERT(frag.compiled() && frag.type() == shader_category::fragment,
             "Invalid fragment shader");

  int succ;
  char log[512];

  _id = glCreateProgram();
  glAttachShader(_id, vert.id());
  glAttachShader(_id, frag.id());
  glLinkProgram(_id);
  glGetProgramiv(_id, GL_LINK_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(vert.id(), 512, nullptr, log);
    SHOGLE_LOG(error, "[ntf::gl_shader_program] Shader program link failed (id: {} -> {})",
               _id, log);
    glDeleteProgram(_id);
    _id = 0;
    return;
  }

  SHOGLE_LOG(verbose, "[ntf::gl_shader_program] Shader program linked (id: {})", _id);
}

void gl_shader_program::load(shader_type vert, shader_type frag, shader_type geom) {
  NTF_ASSERT(_id != 0, "gl_shader_program already initialized");

  NTF_ASSERT(vert.compiled() && vert.type() == shader_category::vertex,
             "Invalid vertex shader");
  NTF_ASSERT(frag.compiled() && frag.type() == shader_category::fragment,
             "Invalid fragment shader");
  NTF_ASSERT(frag.compiled() && geom.type() == shader_category::geometry,
             "Invalid geometry shader");

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
    SHOGLE_LOG(error, "[ntf::gl_shader_program] Shader program link failed (id: {} -> {})",
               _id, log);
    glDeleteProgram(_id);
    _id = 0;
    return;
  } 

  SHOGLE_LOG(verbose, "[ntf::gl_shader_program] Shader program linked (id: {})", _id);
}

void gl_shader_program::load(std::string_view vert_src, std::string_view frag_src) {
  NTF_ASSERT(_id != 0, "gl_shader_program already initialized");
  load(
    shader_type{vert_src, shader_category::vertex},
    shader_type{frag_src, shader_category::fragment}
  );
}

void gl_shader_program::load(std::string_view vert_src, std::string_view frag_src,
                                    std::string_view geom_src) {
  NTF_ASSERT(_id != 0, "gl_shader_program already initialized");
  load(
    shader_type{vert_src, shader_category::vertex},
    shader_type{frag_src, shader_category::fragment},
    shader_type{geom_src, shader_category::geometry}
  );
}

auto gl_shader_program::uniform_location(std::string_view name) const -> uniform_type {
  NTF_ASSERT(linked(), "Invalid gl_shader_program");
  return glGetUniformLocation(_id, name.data());
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

gl_shader_program::~gl_shader_program() noexcept { unload(); }

gl_shader_program::gl_shader_program(gl_shader_program&& p) noexcept : 
  _id(std::move(p._id)) { p._id = 0; }

auto gl_shader_program::operator=(gl_shader_program&& p) noexcept -> gl_shader_program& {
  unload();

  _id = std::move(p._id);

  p._id = 0;

  return *this;
}

} // namespace ntf
