#define SHOGLE_RENDER_SHADER_INL
#include <shogle/render/gl/shader.hpp>
#undef SHOGLE_RENDER_SHADER_INL

namespace ntf {

inline gl::shader::shader(std::string_view src, shader_type type) : _type(type) {
  const auto gltype = renderer::enumtogl(type);
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

inline gl::shader::~shader() noexcept { 
#ifdef SHOGLE_GL_RAII_UNLOAD
  unload();
#endif
}

inline gl::shader::shader(shader&& s) noexcept : 
  _id(std::move(s._id)), _type(std::move(s._type)) { s._id = 0; }

inline auto gl::shader::operator=(shader&& s) noexcept -> shader& {
  unload();

  _id = std::move(s._id);
  _type = std::move(s._type);

  s._id = 0;

  return *this;
}

inline void gl::shader::unload() {
  if (compiled()) {
    SHOGLE_INTERNAL_LOG_FMT(verbose, "[SHOGLE][ntf::gl::shader] Unloaded (id: {})", _id);
    glDeleteShader(_id);
    _id = 0;
  }
}


inline gl::shader_program::shader_program(shader vert, shader frag) {
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
    ntf::log::error("[SHOGLE][ntf::gl::shader_program] Link failed (id: {}) -> {}", _id, log);
    glDeleteProgram(_id);
    _id = 0;
    return;
  }

  SHOGLE_INTERNAL_LOG_FMT(verbose, "[SHOGLE][ntf::gl::shader_program] Linked (id: {})", _id);
}

inline gl::shader_program::shader_program(shader vert, shader frag, shader geom) {
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
    ntf::log::error("[SHOGLE][ntf::gl::shader_program] Link failed (id: {}) -> {}", _id, log);
    glDeleteProgram(_id);
    _id = 0;
    return;
  } 

  SHOGLE_INTERNAL_LOG_FMT(verbose, "[SHOGLE][ntf::gl::shader_program] Linked (id: {})", _id);
}

inline gl::shader_program::~shader_program() noexcept {
#ifdef SHOGLE_GL_RAII_UNLOAD
  unload();
#endif
}

inline gl::shader_program::shader_program(shader_program&& p) noexcept : 
  _id(std::move(p._id)) { p._id = 0; }

inline auto gl::shader_program::operator=(shader_program&& p) noexcept -> shader_program& {
  unload();

  _id = std::move(p._id);

  p._id = 0;

  return *this;
}

inline bool gl::shader_program::uniform_location(shader_uniform& uniform, std::string_view name) const {
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

inline void gl::shader_program::unload() {
  if (_id) {
    SHOGLE_INTERNAL_LOG_FMT(verbose, "[SHOGLE][ntf::gl::shader_program] Unloaded (id: {})", _id);
    glDeleteProgram(_id);
    _id = 0;
  }
}

inline void gl::shader_program::use() const {
  assert(linked() && "Attempted to use unlinked shader program");
  glUseProgram(_id);
}

inline void gl::shader_program::set_uniform(shader_uniform location, const int val) const {
  glUniform1i(location, val);
}

inline void gl::shader_program::set_uniform(shader_uniform location, const float val) const {
  glUniform1f(location, val);
}

inline void gl::shader_program::set_uniform(shader_uniform location, const vec2& val) const {
  glUniform2fv(location, 1, glm::value_ptr(val));
}

inline void gl::shader_program::set_uniform(shader_uniform location, const vec3& val) const {
  glUniform3fv(location, 1, glm::value_ptr(val));
}

inline void gl::shader_program::set_uniform(shader_uniform location, const vec4& val) const {
  glUniform4fv(location, 1, glm::value_ptr(val));
}

inline void gl::shader_program::set_uniform(shader_uniform location, const mat3& val) const {
  glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(val));
}

inline void gl::shader_program::set_uniform(shader_uniform location, const mat4& val) const {
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(val));
}

template<typename T>
bool gl::shader_program::set_uniform(std::string_view name, const T& val) const {
  shader_uniform location{};

  if (!uniform_location(location, name)) {
    return false;
  }

  set_uniform(location, val);
  return true;
}


inline bool gl::uniform_list::register_uniform(const shader_program& shader, std::string_view name, 
                                                        uniform_type type) {
  shader_uniform uniform{};
  if (!shader.uniform_location(uniform, name)) {
    return false;
  }

  _uniforms.emplace_back(header{uniform, type});
  return true;
}

inline void gl::uniform_list::register_uniform(shader_uniform uniform, uniform_type type) {
  _uniforms.emplace_back(header{uniform, type});
}

inline void gl::uniform_list::clear() { _uniforms.clear(); }


inline gl::shader_args::shader_args(const uniform_list& list) {
  size_t data_size = 0;
  for (const auto& header : list.uniforms()) {
    _uniforms.emplace(std::make_pair(header.uniform, std::make_pair(header.type, data_size)));
    data_size += renderer::enumtosz(header.type);
  }
  _data = new uint8_t[data_size];
}

inline gl::shader_args::~shader_args() noexcept { clear(); }

inline gl::shader_args::shader_args(shader_args&& s) noexcept :
  _uniforms{std::move(s._uniforms)}, _data(std::move(s._data)) { s._data = nullptr; }

inline auto gl::shader_args::operator=(shader_args&& s) noexcept -> shader_args& {
  clear();

  _uniforms = std::move(s._uniforms);
  _data = std::move(s._data);

  s._data = nullptr;

  return *this;
}

inline void gl::shader_args::clear() {
  _uniforms.clear();
  if (_data) {
    delete[] _data;
  }
}

inline void gl::shader_args::bind(const shader_program& shader) const {
  shader.use();
  for (const auto& [uniform, pair] : _uniforms) {
    const auto& [type, offset] = pair;
    bind_uniform(shader, uniform, type, offset);
  }
}

inline void gl::shader_args::bind_uniform(const shader_program& shader, shader_uniform location, 
                                                   uniform_type type, size_t off) const {
  switch (type) {
    case uniform_type::scalar: {
      shader.set_uniform(location, *reinterpret_cast<const float*>(_data+off));
      break;
    }
    case uniform_type::iscalar: {
      shader.set_uniform(location, *reinterpret_cast<const int*>(_data+off));
      break;
    };
    case uniform_type::vec2: {
      shader.set_uniform(location, *reinterpret_cast<const vec2*>(_data+off));
      break;
    }
    case uniform_type::vec3: {
      shader.set_uniform(location, *reinterpret_cast<const vec3*>(_data+off));
      break;
    }
    case uniform_type::vec4: {
      shader.set_uniform(location, *reinterpret_cast<const vec4*>(_data+off));
      break;
    }
    case uniform_type::mat3: {
      shader.set_uniform(location, *reinterpret_cast<const mat3*>(_data+off));
      break;
    }
    case uniform_type::mat4: {
      shader.set_uniform(location, *reinterpret_cast<const mat4*>(_data+off));
      break;
    }
  }
}

template<typename T>
bool gl::shader_args::set_uniform(shader_uniform uniform, T&& val) {
  if (_uniforms.find(uniform) == _uniforms.end()) {
    return false;
  }

  const auto& [type, offset] = _uniforms.at(uniform);
  assert(renderer::enumtosz(type) == sizeof(val) && "Attempted to set an invalid uniform type");
  memcpy(_data+offset, &val, sizeof(val));

  return true;
}

template<typename T>
bool gl::shader_args::set_uniform(const shader_program& shader, std::string_view uniform, T&& val) {
  shader_uniform location{};
  if (!shader.uniform_location(location, uniform)) {
    return false;
  }

  return set_uniform(location, std::forward<T>(val));
}

} // namespace ntf
