#include <shogle/render/gl/shader.hpp>

#include <shogle/core/log.hpp>
#include <shogle/core/error.hpp>

namespace ntf::shogle::gl {

shader::shader(std::string src, type type) :
  _src(std::move(src)), _type(type) {}

shader::shader(shader&& s) noexcept :
  _src(std::move(s._src)),
  _shad_id(s._shad_id),
  _type(s._type) { s._shad_id = 0; }

shader& shader::operator=(shader&& s) noexcept {
  auto id = _shad_id;
  if (_shad_id) {
    glDeleteShader(_shad_id);
  }

  _src = std::move(s._src);
  _shad_id = s._shad_id;
  _type = s._type;

  s._shad_id = 0;

  Log::verbose("[gl::shader] Shader overwritten (id: {})", id);
  return *this;
}

shader::~shader() {
  if (!_shad_id) return;
  auto id = _shad_id;
  glDeleteShader(_shad_id);
  Log::verbose("[gl::shader] Shader destroyed (id: {})", id);
}

void shader::compile() {
  int succ;
  char log[512];

  GLenum shader_type;
  switch (_type) {
    case type::vertex: {
      shader_type = GL_VERTEX_SHADER;
      break;
    }
    case type::geometry: {
      shader_type = GL_GEOMETRY_SHADER;
      break;
    }
    case type::fragment: {
      shader_type = GL_FRAGMENT_SHADER;
      break;
    }
  }

  const char* _char_src = _src.c_str();
  _shad_id = glCreateShader(shader_type);
  glShaderSource(_shad_id, 1, &_char_src, nullptr);
  glCompileShader(_shad_id);
  glGetShaderiv(_shad_id, GL_COMPILE_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(_shad_id, 512, nullptr, log);
    glDeleteShader(_shad_id);
    throw ntf::error{"[gl::shader] Shader compilation falied: {}", log};
  }
}

} // namespace ntf::shogle::gl
