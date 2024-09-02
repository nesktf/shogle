#define SHOGLE_RENDER_SHADER_INL
#include <shogle/render/gl/shader.hpp>
#undef SHOGLE_RENDER_SHADER_INL

namespace ntf {

inline gl_renderer::shader::shader(std::string_view src, shader_category type) : _type(type) {
  const auto gltype = renderer_type::enumtogl(type);
  int succ;
  char log[512];

  const char* _char_src = src.data();
  _id = glCreateShader(gltype);
  glShaderSource(_id, 1, &_char_src, nullptr);
  glCompileShader(_id);
  glGetShaderiv(_id, GL_COMPILE_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(_id, 512, nullptr, log);
    ntf::log::error("[SHOGLE][ntf::gl::shader] Compilation failed (id: {}) -> {}", _id, log);
    glDeleteShader(_id);
    _id = 0;
    return;
  }

  SHOGLE_INTERNAL_LOG_FMT(verbose, "[SHOGLE][ntf::gl::shader] Compiled (id: {})", _id);
}

inline gl_renderer::shader::~shader() noexcept { 
#ifdef SHOGLE_GL_RAII_UNLOAD
  unload();
#endif
}

inline gl_renderer::shader::shader(shader&& s) noexcept : 
  _id(std::move(s._id)), _type(std::move(s._type)) { s._id = 0; }

inline auto gl_renderer::shader::operator=(shader&& s) noexcept -> shader& {
  unload();

  _id = std::move(s._id);
  _type = std::move(s._type);

  s._id = 0;

  return *this;
}

inline void gl_renderer::shader::unload() {
  if (compiled()) {
    SHOGLE_INTERNAL_LOG_FMT(verbose, "[SHOGLE][ntf::gl::shader] Unloaded (id: {})", _id);
    glDeleteShader(_id);
    _id = 0;
  }
}


inline gl_renderer::shader_program::shader_program(shader vert, shader frag) {
  assert(vert.compiled() && vert.type() == shader_category::vertex && "Invalid vertex shader");
  assert(frag.compiled() && frag.type() == shader_category::fragment && "Invalid fragment shader");

  int succ;
  char log[512];

  _id = glCreateProgram();
  glAttachShader(_id, vert.id());
  glAttachShader(_id, frag.id());
  glLinkProgram(_id);
  glGetProgramiv(_id, GL_LINK_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(vert.id(), 512, nullptr, log);
    ntf::log::error("[SHOGLE][ntf::gl::shader_program] Link failed (id: {}) -> {}", _id, log);
    glDeleteProgram(_id);
    _id = 0;
    return;
  }

  SHOGLE_INTERNAL_LOG_FMT(verbose, "[SHOGLE][ntf::gl::shader_program] Linked (id: {})", _id);
}

inline gl_renderer::shader_program::shader_program(shader vert, shader frag, shader geom) {
  assert(vert.compiled() && vert.type() == shader_category::vertex && "Invalid vertex shader");
  assert(frag.compiled() && frag.type() == shader_category::fragment && "Invalid fragment shader");
  assert(geom.compiled() && geom.type() == shader_category::geometry && "Invalid geometry shader");

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
    ntf::log::error("[SHOGLE][ntf::gl::shader_program] Link failed (id: {}) -> {}", _id, log);
    glDeleteProgram(_id);
    _id = 0;
    return;
  } 

  SHOGLE_INTERNAL_LOG_FMT(verbose, "[SHOGLE][ntf::gl::shader_program] Linked (id: {})", _id);
}

inline gl_renderer::shader_program::~shader_program() noexcept {
#ifdef SHOGLE_GL_RAII_UNLOAD
  unload();
#endif
}

inline gl_renderer::shader_program::shader_program(shader_program&& p) noexcept : 
  _id(std::move(p._id)) { p._id = 0; }

inline auto gl_renderer::shader_program::operator=(shader_program&& p) noexcept -> shader_program& {
  unload();

  _id = std::move(p._id);

  p._id = 0;

  return *this;
}

inline bool gl_renderer::shader_program::uniform_location(uniform_type& uniform, std::string_view name) const {
  if (!linked()) {
    return false;
  }

  const auto loc = glGetUniformLocation(_id, name.data());
  if (loc == -1) {
    return false;
  }

  uniform = loc;
  return true;
}

inline void gl_renderer::shader_program::unload() {
  if (_id) {
    SHOGLE_INTERNAL_LOG_FMT(verbose, "[SHOGLE][ntf::gl::shader_program] Unloaded (id: {})", _id);
    glDeleteProgram(_id);
    _id = 0;
  }
}

inline void gl_renderer::shader_program::use() const {
  assert(linked() && "Attempted to use unlinked shader program");
  glUseProgram(_id);
}

inline void gl_renderer::shader_program::set_uniform(uniform_type location, const int val) const {
  glUniform1i(location, val);
}

inline void gl_renderer::shader_program::set_uniform(uniform_type location, const float val) const {
  glUniform1f(location, val);
}

inline void gl_renderer::shader_program::set_uniform(uniform_type location, const vec2& val) const {
  glUniform2fv(location, 1, glm::value_ptr(val));
}

inline void gl_renderer::shader_program::set_uniform(uniform_type location, const vec3& val) const {
  glUniform3fv(location, 1, glm::value_ptr(val));
}

inline void gl_renderer::shader_program::set_uniform(uniform_type location, const vec4& val) const {
  glUniform4fv(location, 1, glm::value_ptr(val));
}

inline void gl_renderer::shader_program::set_uniform(uniform_type location, const mat3& val) const {
  glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(val));
}

inline void gl_renderer::shader_program::set_uniform(uniform_type location, const mat4& val) const {
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(val));
}

template<typename T>
bool gl_renderer::shader_program::set_uniform(std::string_view name, const T& val) const {
  uniform_type location{};

  if (!uniform_location(location, name)) {
    return false;
  }

  set_uniform(location, val);
  return true;
}

} // namespace ntf
