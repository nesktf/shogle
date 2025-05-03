#include "./state.hpp"

namespace ntf {

GLenum gl_state::shader_type_cast(r_shader_type type) noexcept {
  switch (type) {
    case r_shader_type::vertex:               return GL_VERTEX_SHADER;
    case r_shader_type::fragment:             return GL_FRAGMENT_SHADER;
    case r_shader_type::geometry:             return GL_GEOMETRY_SHADER;
    case r_shader_type::tesselation_eval:     return GL_TESS_EVALUATION_SHADER;
    case r_shader_type::tesselation_control:  return GL_TESS_CONTROL_SHADER;

    case r_shader_type::compute: break;
  }

  NTF_UNREACHABLE();
}

auto gl_state::create_shader(r_shader_type type, std::string_view src) -> shader_t {
  const GLenum gltype = shader_type_cast(type);
  GLenum id = GL_CALL_RET(glCreateShader, gltype);

  const char* src_data = src.data();
  const GLint len = src.size();
  GL_CALL(glShaderSource, id, 1, &src_data, &len);
  GL_CALL(glCompileShader, id);

  int succ;
  GL_CALL(glGetShaderiv, id, GL_COMPILE_STATUS, &succ);
  if (!succ) {
    GLint err_len = 0; // includes null terminator
    GL_CALL(glGetShaderiv, id, GL_INFO_LOG_LENGTH, &err_len);
    std::string log;
    log.resize(err_len);

    GL_CALL(glGetShaderInfoLog, id, 1024, &err_len, log.data());
    GL_CALL(glDeleteShader, id);
    throw error<>::format({"Failed to compile shader: {}"}, log);
  }

  shader_t shader;
  shader.id = id;
  shader.type = gltype;
  return shader;
}

void gl_state::destroy_shader(const shader_t& shader) noexcept {
  NTF_ASSERT(shader.id);
  GL_CALL(glDeleteShader, shader.id);
}

} // namespace ntf
