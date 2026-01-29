#pragma once

#ifndef SHOGLE_GLAPI
#define SHOGLE_GLAPI
#endif

#ifndef SHOGLE_GLAPI_ENTRY
#define SHOGLE_GLAPI_ENTRY
#endif

#if defined(SHOGLE_USE_SYSTEM_GL) && SHOGLE_USE_SYSTEM_GL
#include <GL/gl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct shogle_gl_ver {
  int maj;
  int min;
} shogle_gl_ver;

SHOGLE_GLAPI_ENTRY void shogle_gl_get_version(const char* ver_str, shogle_gl_ver* ver);

#if !defined(SHOGLE_USE_SYSTEM_GL) || !SHOGLE_USE_SYSTEM_GL
#include <KHR/khrplatform.h>
#include <shogle/render/gl/gldefs.h>

#define GL_NO_ERROR                      0x0000
#define GL_INVALID_ENUM                  0x0500
#define GL_INVALID_VALUE                 0x0501
#define GL_INVALID_OPERATION             0x0502
#define GL_OUT_OF_MEMORY                 0x0505
#define GL_STACK_UNDERFLOW               0x0504
#define GL_STACK_OVERFLOW                0x0503
#define GL_INVALID_FRAMEBUFFER_OPERATION 0x0506

#define GL_VERSION  0x1F02
#define GL_RENDERER 0x1F01
#define GL_VENDOR   0x1F00

#define GL_FRAMEBUFFER      0x8D40
#define GL_RENDERBUFFER     0x8D41
#define GL_READ_FRAMEBUFFER 0x8CA8
#define GL_DRAW_FRAMEBUFFER 0x8CA9

#define GL_TEXTURE_1D               0x0DE0
#define GL_TEXTURE_2D               0x0DE1
#define GL_TEXTURE_3D               0x806F
#define GL_TEXTURE_CUBE_MAP         0x8513
#define GL_COLOR_ATTACHMENT0        0x8CE0
#define GL_DEPTH_STENCIL_ATTACHMENT 0x821A
#define GL_STENCIL_ATTACHMENT       0x8D20
#define GL_DEPTH_ATTACHMENT         0x8D00
#define GL_DEPTH_COMPONENT          0x1902

#define SHOGLE_GL_DOFUNCS(X)                                                                      \
  X(glGetString, const GLubyte*, GLenum name)                                                     \
  X(glGetError, GLenum, void)                                                                     \
  X(glGenTextures, void, GLsizei n, GLuint* textures)                                             \
  X(glBindTexture, void, GLenum target, GLuint texture)                                           \
  X(glDeleteTextures, void, GLsizei n, const GLuint* textures)                                    \
  X(glTexImage1D, void, GLenum target, GLint level, GLint internalformat, GLsizei width,          \
    GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels)                 \
  X(glTexStorage1D, void, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)    \
  X(glTexSubImage1D, void, GLenum target, GLint level, GLint xoffset, GLsizei width,              \
    GLenum format, GLenum type, const void* pixels)                                               \
  X(glTexImage2D, void, GLenum target, GLint level, GLint internalformat, GLsizei width,          \
    GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels)                 \
  X(glTexStorage2D, void, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width,    \
    GLsizei height)                                                                               \
  X(glTexImage2DMultisample, void, GLenum target, GLsizei samples, GLenum internalformat,         \
    GLsizei width, GLsizei height, GLboolean fixedsamplelocations)                                \
  X(glTexStorage2DMultisample, void, GLenum target, GLsizei samples, GLenum internalformat,       \
    GLsizei width, GLsizei height, GLboolean fixedsamplelocations)                                \
  X(glTexSubImage2D, void, GLenum target, GLint level, GLint xoffset, GLint yoffset,              \
    GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)                \
  X(glTexImage3D, void, GLenum target, GLint level, GLint internalformat, GLsizei width,          \
    GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels)  \
  X(glTexStorage3D, void, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width,    \
    GLsizei height, GLsizei depth)                                                                \
  X(glTexStorage3DMultisample, void, GLenum target, GLsizei samples, GLenum internalformat,       \
    GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)                 \
  X(glTexSubImage3D, void, GLenum target, GLint level, GLint xoffset, GLint yoffset,              \
    GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type,      \
    const void* pixels)                                                                           \
  X(glTexImage3DMultisample, void, GLenum target, GLsizei samples, GLenum internalformat,         \
    GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)                 \
  X(glTexBuffer, void, GLenum target, GLenum internalformat, GLuint buffer)                       \
  X(glTexBufferRange, void, GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, \
    GLsizeiptr size)                                                                              \
  X(glTexParameteri, void, GLenum target, GLenum pname, GLint param)                              \
  X(glGenerateMipmap, void, GLenum target)                                                        \
  X(glGenBuffers, void, GLsizei n, GLuint* buffers)                                               \
  X(glBindBuffer, void, GLenum target, GLuint buffer)                                             \
  X(glDeleteBuffers, void, GLsizei n, const GLuint* buffers)                                      \
  X(glBufferData, void, GLenum target, GLsizeiptr size, const void* data, GLenum usage)           \
  X(glBufferSubData, void, GLenum target, GLintptr offset, GLsizeiptr size, const void* data)     \
  X(glMapBuffer, void*, GLenum target, GLenum access)                                             \
  X(glMapBufferRange, void*, GLenum target, GLintptr offset, GLsizeiptr length,                   \
    GLbitfield access)                                                                            \
  X(glUnmapBuffer, void, GLenum target)                                                           \
  X(glCreateShader, GLuint, GLenum type)                                                          \
  X(glDeleteShader, void, GLuint shader)                                                          \
  X(glShaderSource, void, GLuint shader, GLsizei count, const GLchar* const* string,              \
    const GLint* length)                                                                          \
  X(glCompileShader, void, GLuint shader)                                                         \
  X(glGetShaderiv, void, GLuint shader, GLenum pname, GLint* params)                              \
  X(glGetShaderInfoLog, void, GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog)   \
  X(glBindRenderbuffer, void, GLenum target, GLuint renderbuffer)                                 \
  X(glGenRenderbuffers, void, GLsizei n, GLuint* renderbuffers)                                   \
  X(glDeleteRenderbuffers, void, GLsizei n, const GLuint* renderbuffers)                          \
  X(glRenderbufferStorage, void, GLenum target, GLenum internalformat, GLsizei width,             \
    GLsizei height)                                                                               \
  X(glDeleteFramebuffers, void, GLsizei n, GLuint* framebuffers)                                  \
  X(glGenFramebuffers, void, GLsizei n, GLuint* framebuffers)                                     \
  X(glBindFramebuffer, void, GLenum target, GLuint framebuffer)                                   \
  X(glCheckFramebufferStatus, GLenum, GLenum target)                                              \
  X(glFramebufferTexture1D, void, GLenum target, GLenum attachment, GLenum textarget,             \
    GLuint texture, GLint level)                                                                  \
  X(glFramebufferTexture2D, void, GLenum target, GLenum attachment, GLenum textarget,             \
    GLuint texture, GLint level)                                                                  \
  X(glFramebufferTexture3D, void, GLenum target, GLenum attachment, GLenum textarget,             \
    GLuint texture, GLint level, GLint zoffset)                                                   \
  X(glFramebufferRenderbuffer, void, GLenum target, GLenum attachment, GLenum renderbuffertarget, \
    GLuint renderbuffer)                                                                          \
  X(glBlitFramebuffer, void, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0,     \
    GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)                        \
  X(glCreateVertexArrays, void, GLsizei n, GLuint* arrays)                                        \
  X(glDeleteVertexArrays, void, GLsizei n, const GLuint* arrays)                                  \
  X(glPixelStorei, void, GLenum pname, GLint param)

#define SHOGLE_GL_DECLPROC(name_, ret_, ...) \
  typedef ret_(SHOGLE_GLAPI_ENTRY* PFN_shogle_##name_)(__VA_ARGS__);

#define SHOGLE_GL_DEFPROC(name_, ...) PFN_shogle_##name_ name_;

SHOGLE_GL_DECLPROC(glDebugProc, void, GLenum src, GLenum type, GLenum id, GLenum severity,
                   GLsizei length, const char* message, const void* user)

SHOGLE_GL_DOFUNCS(SHOGLE_GL_DECLPROC)

typedef struct shogle_gl_functions {
  SHOGLE_GL_DOFUNCS(SHOGLE_GL_DEFPROC)
} shogle_gl_functions;

typedef void* (*PFN_shogle_glGetProcAddress)(const char*);

typedef enum shogle_gl_load_ret {
  SHOGLE_GL_LOAD_NO_ERROR = 0,
  SHOGLE_GL_LOAD_NO_FUNC = 1,
  SHOGLE_GL_LOAD_NO_GL = 2,
  SHOGLE_GL_LOAD_VER_MISMATCH = 3,

  _SHOGLE_GL_LOAD_FORCE_32BIT = 0x7FFFFFFF,
} shogle_gl_load_ret;

SHOGLE_GLAPI_ENTRY shogle_gl_load_ret shogle_gl_load_funcs(PFN_shogle_glGetProcAddress proc,
                                                           shogle_gl_functions* funcs,
                                                           shogle_gl_ver* ver);

#endif

#ifdef __cplusplus
}
#endif
