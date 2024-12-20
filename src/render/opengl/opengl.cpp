#include "./opengl.hpp"

namespace ntf {

GLenum gl_check_error(const char* file, int line) {
  GLenum err{};
  while ((err = glGetError()) != GL_NO_ERROR) {
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
    SHOGLE_LOG(error, "[ntf::gl_check_error] GL error ({}) | \"{}\":{} -> {}",
               err, file, line, err_str);
  }
  return err;
}

void gl_clear_bits(r_clear clear, const color4& color) {
  GLbitfield clear_bits{0};
  if (+(clear & r_clear::color)) {
    glClearColor(color.r, color.g, color.b, color.a);
    clear_bits |= GL_COLOR_BUFFER_BIT;
  }
  if (+(clear & r_clear::depth)) {
    clear_bits |= GL_DEPTH_BUFFER_BIT;
  }
  if (+(clear & r_clear::stencil)) {
    clear_bits |= GL_STENCIL_BUFFER_BIT;
  }
  if (clear_bits != 0) {
    glClear(clear_bits);
  }
}

} // namespace ntf
