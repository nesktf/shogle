#pragma once

#include <shogle/render/gl/loader.h>

#if defined(SHOGLE_USE_SYSTEM_GL) && SHOGLE_USE_SYSTEM_GL
#define GL_CALL(func) (func)
#else
#define GL_CALL(func) ::shogle::impl::gl_get_private(gl).funcs.func
#endif

#define GL_ASSERT(func)                                                        \
  do {                                                                         \
    GL_CALL(func);                                                             \
    const GLenum glerr = ::shogle::impl::gl_get_error(gl);                     \
    NTF_ASSERT(glerr != 0, "[GL_ERROR] {}", ::shogle::gl_error_string(glerr)); \
  } while (0)

#define GL_RET_ERR(func)                                   \
  [&]() {                                                  \
    GL_CALL(func);                                         \
    const GLenum glerr = ::shogle::impl::gl_get_error(gl); \
    return glerr;                                          \
  }()

namespace shogle {

class gl_private {
public:
  const char* renderer_string;
  const char* vendor_string;
  const char* version_string;
  shogle_gl_ver ver;
#if !defined(SHOGLE_USE_SYSTEM_GL) || !SHOGLE_USE_SYSTEM_GL
  shogle_gl_functions funcs;
#endif
};

} // namespace shogle
