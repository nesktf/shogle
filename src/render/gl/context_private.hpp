#pragma once

#include <shogle/render/gl/loader.h>

#include <shogle/util/memory.hpp>

#include <shogle/render/gl/common.hpp>

#if defined(SHOGLE_USE_SYSTEM_GL) && SHOGLE_USE_SYSTEM_GL
#define GL_CALL(func) (func)
#else
#define GL_CALL(func) ::shogle::impl::gl_get_private(gl).funcs.func
#endif

#define GL_ASSERT(func)                                                 \
  do {                                                                  \
    GL_CALL(func);                                                      \
    const GLenum glerr = gl.get_error();                                \
    SHOGLE_ASSERT(glerr == 0, ::shogle::gl_error_string(glerr).data()); \
  } while (0)

#define GL_ASSERT_RET(func)                                             \
  [&]() {                                                               \
    const auto ret = GL_CALL(func);                                     \
    const GLenum glerr = gl.get_error();                                \
    SHOGLE_ASSERT(glerr == 0, ::shogle::gl_error_string(glerr).data()); \
    return ret;                                                         \
  }();

#define GL_RET_ERR(func)                 \
  [&]() {                                \
    GL_CALL(func);                       \
    const GLenum glerr = gl.get_error(); \
    return glerr;                        \
  }()

namespace shogle {

class gl_private {
public:
  gl_private(mem::scratch_arena&& arena_, const gl_surface_provider& surf_prov_) noexcept :
      arena(std::move(arena_)), surf_prov(surf_prov_) {}

public:
  mem::scratch_arena arena;
  const char* renderer_string;
  const char* vendor_string;
  const char* version_string;
  shogle_gl_ver ver;
#if !defined(SHOGLE_USE_SYSTEM_GL) || !SHOGLE_USE_SYSTEM_GL
  shogle_gl_functions funcs;
#endif
  gl_surface_provider surf_prov;
};

} // namespace shogle
