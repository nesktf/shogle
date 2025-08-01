#include "./context.hpp"

namespace shogle {

GLenum gl_state::shader_type_cast(shader_type type) noexcept {
  switch (type) {
    case shader_type::vertex:               return GL_VERTEX_SHADER;
    case shader_type::fragment:             return GL_FRAGMENT_SHADER;
    case shader_type::geometry:             return GL_GEOMETRY_SHADER;
    case shader_type::tesselation_eval:     return GL_TESS_EVALUATION_SHADER;
    case shader_type::tesselation_control:  return GL_TESS_CONTROL_SHADER;
    case shader_type::compute:              return GL_COMPUTE_SHADER;
  }

  NTF_UNREACHABLE();
}

ctx_status gl_state::create_shader(glshader_t& shader, shader_type type,
                                        std::string_view src, shad_err_str& err)
{
  const GLenum gltype = shader_type_cast(type);
  GLenum id = GL_CALL_RET(glCreateShader, gltype);

  const char* src_data = src.data();
  const GLint len = src.size();
  GL_CALL(glShaderSource, id, 1, &src_data, &len);
  glCompileShader(id); // Avoid assertions

  int succ;
  GL_CALL(glGetShaderiv, id, GL_COMPILE_STATUS, &succ);
  if (!succ) {
    GLint err_len = 0; // includes null terminator
    GL_CALL(glGetShaderiv, id, GL_INFO_LOG_LENGTH, &err_len);
    auto span = _alloc.arena_span<char>(static_cast<size_t>(err_len));
    GL_CALL(glGetShaderInfoLog, id, 1024, &err_len, span.data());
    GL_CALL(glDeleteShader, id);
    err = {span.data(), span.size()};
    return render_error::pip_compilation_failure;
  }

  shader.id = id;
  shader.type = gltype;
  return render_error::no_error;
}

void gl_state::destroy_shader(glshader_t& shader) {
  NTF_ASSERT(shader.id);
  GL_CALL(glDeleteShader, shader.id);
}

ctx_status gl_context::create_shader(ctx_shad& shad, shad_err_str& err,
                                          const ctx_shad_desc& desc) {
  ctx_shad handle = _shaders.acquire();
  NTF_ASSERT(check_handle(handle));
  auto& shader = _shaders.get(handle);
  shader.id = 0;
  const auto status = _state.create_shader(shader, desc.type, desc.source, err);
  if (status != render_error::no_error) {
    _shaders.push(handle);
    return status;
  }
  NTF_ASSERT(shader.id);
  shad = handle;
  return status;
}

ctx_status gl_context::destroy_shader(ctx_shad shad) noexcept {
  if (!_shaders.validate(shad)) {
    return render_error::invalid_handle;
  }
  auto& shader = _shaders.get(shad);
  _state.destroy_shader(shader);
  _shaders.push(shad);
  return render_error::no_error;
}

} // namespace shogle
