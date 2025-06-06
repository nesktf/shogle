#include "./context.hpp"

namespace ntf::render {

GLenum gl_state::attrib_underlying_type_cast(attribute_type type) noexcept {
  switch (type) {
    case attribute_type::f32:   [[fallthrough]];
    case attribute_type::vec2:  [[fallthrough]];
    case attribute_type::vec3:  [[fallthrough]];
    case attribute_type::vec4:  [[fallthrough]];
    case attribute_type::mat3:  [[fallthrough]];
    case attribute_type::mat4:  return GL_FLOAT;

    case attribute_type::f64:   [[fallthrough]];
    case attribute_type::dvec2: [[fallthrough]];
    case attribute_type::dvec3: [[fallthrough]];
    case attribute_type::dvec4: [[fallthrough]];
    case attribute_type::dmat3: [[fallthrough]];
    case attribute_type::dmat4: return GL_DOUBLE;

    case attribute_type::i32:   [[fallthrough]];
    case attribute_type::ivec2: [[fallthrough]];
    case attribute_type::ivec3: [[fallthrough]];
    case attribute_type::ivec4: return GL_INT;
  }

  NTF_UNREACHABLE();
}

GLbitfield gl_state::clear_bit_cast(clear_flag flags) noexcept {
  GLbitfield clear_bits{0};
  if (+(flags & clear_flag::color)) {
    clear_bits |= GL_COLOR_BUFFER_BIT;
  }
  if (+(flags & clear_flag::depth)) {
    clear_bits |= GL_DEPTH_BUFFER_BIT;
  }
  if (+(flags & clear_flag::stencil)) {
    clear_bits |= GL_STENCIL_BUFFER_BIT;
  }
  return clear_bits;
}

GLenum gl_state::check_error(std::string_view file, int line) noexcept {
  GLenum out = GL_NO_ERROR;
  GLenum err{};
  while ((err = glGetError()) != GL_NO_ERROR) {
    out = err;
    const char* err_str;
    switch (err) {
      case GL_INVALID_ENUM: { err_str = "INVALID_ENUM"; break; }
      case GL_INVALID_VALUE: { err_str = "INVALID_VALUE"; break; }
      case GL_INVALID_OPERATION: { err_str = "INVALID_OPERATION"; break; }
      case GL_STACK_OVERFLOW: { err_str = "STACK_OVERFLOW"; break; }
      case GL_STACK_UNDERFLOW: { err_str = "STACK_UNDERFLOW"; break; }
      case GL_OUT_OF_MEMORY: { err_str = "OUT_OF_MEMORY"; break; }
      case GL_INVALID_FRAMEBUFFER_OPERATION: { err_str = "INVALID_FRAMEBUFFER_OPERATION"; break; }
      default: { err_str = "UNKNOWN_ERROR"; break; }
    }
    SHOGLE_LOG(error, "[{}:{}] GL ERROR ({}): {}", file, line, err, err_str);
  }
  return out;
}

gl_state::gl_state(ctx_alloc& alloc) noexcept :
  _alloc{alloc},
  _bound_vao{0},
  _bound_program{0},
  _bound_texs{alloc.make_adaptor<std::pair<GLuint, GLenum>>()},
  _active_tex{0}
{
  std::memset(_bound_buffers, NULL_BINDING, GLBUFFER_TYPE_COUNT*sizeof(GLuint));
  std::memset(_bound_fbos, DEFAULT_FBO, GLFBO_BIND_COUNT*sizeof(GLuint));
}

void gl_state::init(GLDEBUGPROC debug_callback, gl_context* ctx) {
  GLint max_tex;
  GL_CALL(glGetIntegerv, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_tex);
  _bound_texs.resize(max_tex, std::make_pair(NULL_BINDING, GL_TEXTURE_2D));

  // State cleanup
  GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, DEFAULT_FBO);
  GL_CALL(glUseProgram, NULL_BINDING);
  GL_CALL(glBindVertexArray, NULL_BINDING);
  GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, NULL_BINDING);
  GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, NULL_BINDING);
  GL_CALL(glBindBuffer, GL_UNIFORM_BUFFER, NULL_BINDING);
  GL_CALL(glBindBuffer, GL_TEXTURE_BUFFER, NULL_BINDING);
  GL_CALL(glBindBuffer, GL_SHADER_STORAGE_BUFFER, NULL_BINDING);
  for (GLint i = max_tex-1; i >= 0; --i) {
    // Do it in reverse to end up with GL_TEXTURE0 active
    GL_CALL(glActiveTexture, GL_TEXTURE0+i);
    GL_CALL(glBindTexture, GL_TEXTURE_2D, NULL_BINDING);
  }

  if (debug_callback) {
    NTF_ASSERT(ctx);
    GL_CALL(glEnable, GL_DEBUG_OUTPUT);
    GL_CALL(glDebugMessageCallback, debug_callback, ctx);
  }
}

void gl_state::create_vao(glvao_t& vao) {
  GLuint id;
  GL_CALL(glGenVertexArrays, 1, &id);
  vao.id = id;
}

void gl_state::destroy_vao(glvao_t& vao) {
  NTF_ASSERT(vao.id);
  GLuint id = vao.id;
  if (_bound_vao == id) {
    _bound_vao = NULL_BINDING;
    GL_CALL(glBindVertexArray, NULL_BINDING);
  }
  GL_CALL(glDeleteVertexArrays, 1, &id);
}

bool gl_state::vao_bind(GLuint id) {
  if (_bound_vao == id) {
    return false;
  }
  GL_CALL(glBindVertexArray, id);
  _bound_vao = id;
  return true;
}

} // namespace ntf::render
