// Shitty opengl loader based on glad but not using global state
// see render/gl/loader.h for function definitions
#include <shogle/render/gl/loader.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define GLLOAD(name_) \
  funcs->name_ = (PFN_shogle_##name_)proc(#name_)

#define GLLOADCHECK(name_) \
  (GLLOAD(name_)) == NULL

#ifdef NDEBUG
#define GLLOADREQ(name_) \
  if (GLLOADCHECK(name_)) { \
    return SHOGLE_GL_LOAD_NO_FUNC; \
  }
#else
#define GLLOADREQ(name_, ...) \
  GLLOAD(name_); \
  assert(funcs->name_);
#endif

#define CHECK_VER(maj_, min_) \
  ((ver->maj == maj_ && ver->min >= min_) || ver->maj > maj_)

#define SHOGLE_GL_REQUIRE_MIN 3
#define SHOGLE_GL_REQUIRE_MAJ 3

void shogle_gl_get_version(const char* ver_str, shogle_gl_ver* ver) {
  assert(ver_str && ver);

  /* Thank you @elmindreda
   * https://github.com/elmindreda/greg/blob/master/templates/greg.c.in#L176
   * https://github.com/glfw/glfw/blob/master/src/context.c#L36
   */
  const char* prefixes[] = {
    "OpenGL ES-CM ",
    "OpenGL ES-CL ",
    "OpenGL ES ",
    NULL
  };

  for (int i = 0;  prefixes[i];  i++) {
    const size_t length = strlen(prefixes[i]);
    if (strncmp(ver_str, prefixes[i], length) == 0) {
      ver_str += length;
      break;
    }
  }

#ifdef _MSC_VER
  sscanf_s(ver_str, "%d.%d", &ver->maj, &ver->,min);
#else
  sscanf(ver_str, "%d.%d", &ver->maj, &ver->min);
#endif
}

#if !defined(SHOGLE_USE_SYSTEM_GL) || !SHOGLE_USE_SYSTEM_GL
shogle_gl_load_ret shogle_gl_load_funcs(PFN_shogle_glGetProcAddress proc,
                                        shogle_gl_functions* funcs, shogle_gl_ver* ver) {
  assert(proc && funcs && ver);

  if (GLLOADCHECK(glGetString)) {
    return SHOGLE_GL_LOAD_NO_FUNC;
  }

  const char* version = (const char*)funcs->glGetString(GL_VERSION);
  if (!version) {
    return SHOGLE_GL_LOAD_NO_GL;
  }
  shogle_gl_get_version(version, ver);

  if (!CHECK_VER(SHOGLE_GL_REQUIRE_MAJ, SHOGLE_GL_REQUIRE_MIN)) {
    return SHOGLE_GL_LOAD_VER_MISMATCH;
  }

  SHOGLE_GL_DOFUNCS(GLLOADREQ)

  return SHOGLE_GL_LOAD_NO_ERROR;
}
#endif
