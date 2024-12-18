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

} // namespace ntf
