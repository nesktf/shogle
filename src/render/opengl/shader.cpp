#include "./shader.hpp"

namespace ntf {

void gl_shader::load(r_shader_type type, std::string_view src) {
  NTF_ASSERT(!_id);

  const char* src_data = src.data();
  const GLenum gltype = gl_shader_type_cast(type);
  NTF_ASSERT(gltype);

  int succ;
  GLuint id = glCreateShader(gltype);
  glShaderSource(id, 1, &src_data, nullptr);
  glCompileShader(id);
  glGetShaderiv(id, GL_COMPILE_STATUS, &succ);
  if (!succ) {
    // TODO: Store (or pass) the log somewhere
    char log[512]; 
    glGetShaderInfoLog(id, 512, nullptr, log);
    SHOGLE_LOG(error, "[ntf::gl_shader] Shader compilation failed (id: {}) -> {}", id, log);
    glDeleteShader(id);
    return;
  }

  _id = id;
  _type = type;
}

void gl_shader::unload() {
  NTF_ASSERT(_id);

  glDeleteShader(_id);

  _id = 0;
  _type = r_shader_type::none;
}

} // namespace ntf
