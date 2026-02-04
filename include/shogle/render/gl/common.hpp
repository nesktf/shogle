#pragma once

#include <shogle/render/common.hpp>

// TODO: Do something to avoid polluting the root namespace with this?
#include <KHR/khrplatform.h>

#include <string>

#ifndef SHOGLE_DISABLE_INTERNAL_LOGS

#define OPENGL_ALLOC_LOG(fmt_, ...)  SHOGLE_RENDER_LOG(verbose, "OpenGL", fmt_, __VA_ARGS__)
#define OPENGL_PROP_LOG(fmt_, ...)   SHOGLE_RENDER_LOG(verbose, "OpenGL", fmt_, __VA_ARGS__)
#define OPENGL_ACTION_LOG(fmt_, ...) SHOGLE_RENDER_LOG(verbose, "OpenGL", fmt_, __VA_ARGS__)
#define OPENGL_ERR_LOG(fmt_, ...)    SHOGLE_RENDER_LOG(error, "OpenGL", fmt_, __VA_ARGS__)
#define OPENGL_WARN_LOG(fmt_, ...)   SHOGLE_RENDER_LOG(warning, "OpenGL", fmt_, __VA_ARGS__)
#else
#define OPENGL_ALLOC_LOG(...)
#define OPENGL_PROP_LOG(...)
#define OPENGL_ACTION_LOG(...)
#define OPENGL_ERR_LOG(...)
#define OPENGL_WARN_LOG(...)
#endif

namespace shogle {

namespace gldefs {

// Try to not pollute the root namespace with opengl things
#include <shogle/render/gl/gldefs.h>
typedef GLuint GLhandle; // just because

} // namespace gldefs

class gl_private;
class gl_context;
class gl_texture;
class gl_buffer;
class gl_shader;
class gl_graphics_pipeline;
class gl_vertex_layout;
class gl_framebuffer;

namespace impl {

scratch_arena& gl_get_scratch_arena(gl_context& gl);
gl_private& gl_get_private(gl_context& gl);
gldefs::GLenum gl_get_error(gl_context& gl);

} // namespace impl

constexpr gldefs::GLhandle GL_DEFAULT_BINDING = 0;
constexpr gldefs::GLhandle GL_NULL_HANDLE = std::numeric_limits<gldefs::GLhandle>::max();

std::string_view gl_error_string(gldefs::GLenum err) noexcept;

using PFN_glGetProcAddress = void* (*)(const char* name);

struct gl_surface_provider {
  virtual ~gl_surface_provider() = default;
  virtual PFN_glGetProcAddress proc_loader() noexcept = 0;
  virtual extent2d surface_extent() const noexcept = 0;
  virtual void swap_buffers() noexcept = 0;
};

struct gl_version {
  u32 major;
  u32 minor;
};

class gl_sv_error : public std::exception {
public:
  gl_sv_error(const char* msg, gldefs::GLenum err) noexcept : _msg(msg), _err(err) {}

  gl_sv_error(const char* msg) noexcept : _msg(msg), _err(0) {}

public:
  const char* what() const noexcept override { return _msg; }

  gldefs::GLenum code() const noexcept { return _err; }

  std::string_view code_str() const noexcept { return gl_error_string(_err); }

private:
  const char* _msg;
  gldefs::GLenum _err;
};

class gl_s_error : public std::exception {
public:
  gl_s_error(std::string msg, gldefs::GLenum err) : _msg(std::move(msg)), _err(err) {}

  gl_s_error(std::string msg) : _msg(std::move(msg)), _err(0) {}

public:
  const char* what() const noexcept override { return _msg.c_str(); }

  gldefs::GLenum code() const noexcept { return _err; }

  std::string_view code_str() const noexcept { return gl_error_string(_err); }

private:
  std::string _msg;
  gldefs::GLenum _err;
};

template<typename T>
using gl_sv_expect = ntf::expected<T, gl_sv_error>;

template<typename T>
using gl_s_expect = ntf::expected<T, gl_s_error>;

template<typename T>
using gl_expect = ntf::expected<T, gldefs::GLenum>;

template<typename T>
class gl_raii_wrapper {
public:
  gl_raii_wrapper(gl_context& gl, T&& obj) : _gl(&gl), _obj(std::move(obj)) {}

  ~gl_raii_wrapper() { reset(); }

  gl_raii_wrapper(gl_raii_wrapper&& other) noexcept = default;
  gl_raii_wrapper(const gl_raii_wrapper& other) noexcept = delete;

public:
  [[nodiscard]] T release() {
    NTF_ASSERT(!empty());
    auto obj = *std::move(_obj);
    _obj.reset();
    return obj;
  }

  void reset() {
    if (!empty()) {
      _obj->destroy(*_gl);
      _obj.reset();
    }
  }

  bool empty() const { return !_obj.has_value(); }

public:
  operator T&() { return **this; }

  operator const T&() { return **this; }

  T* operator->() { return &*_obj; }

  const T* operator->() const { return &*_obj; }

  T& operator*() { return *_obj; }

  const T& operator*() const { return *_obj; }

  gl_raii_wrapper& operator=(gl_raii_wrapper&& other) noexcept = default;
  gl_raii_wrapper& operator=(const gl_raii_wrapper&& other) noexcept = delete;

private:
  gl_context* _gl;
  ntf::nullable<T> _obj;
};

namespace meta {

template<>
struct renderer_traits<gl_context> {
  static constexpr bool is_specialized = true;
  static constexpr render_context_tag tag = render_context_tag::opengl;
};

template<typename T>
struct gl_data_traits {
  static constexpr bool is_specialized = false;
};

template<typename T>
concept gl_data_type = gl_data_traits<T>::is_specialized;

#define SHOGLE_DEFINE_GL_DATA_TRAIT(type_, tag_)             \
  template<>                                                 \
  struct gl_data_traits<type_> {                             \
    static constexpr bool is_specialized = true;             \
    static constexpr ::shogle::gldefs::GLenum gl_tag = tag_; \
  }

SHOGLE_DEFINE_GL_DATA_TRAIT(i8, 0x1400);  // GL_BYTE
SHOGLE_DEFINE_GL_DATA_TRAIT(u8, 0x1401);  // GL_UNSIGNED_BYTE
SHOGLE_DEFINE_GL_DATA_TRAIT(i16, 0x1402); // GL_SHORT
SHOGLE_DEFINE_GL_DATA_TRAIT(u16, 0x1403); // GL_SHORT
SHOGLE_DEFINE_GL_DATA_TRAIT(i32, 0x1404); // GL_INT
SHOGLE_DEFINE_GL_DATA_TRAIT(u32, 0x1405); // GL_UNSIGNED_INT
SHOGLE_DEFINE_GL_DATA_TRAIT(f32, 0x1406); // GL_FLOAT
// SHOGLE_DEFINE_GL_DATA_TRAIT(f16, 0x140B); // GL_HALF_FLOAT
// SHOGLE_DEFINE_GL_DATA_TRAIT(depth_stencil_u32, 0x84FA); // GL_UNSIGNED_INT_24_8

#undef SHOGLE_DEFINE_GL_DATA_TRAIT

} // namespace meta

} // namespace shogle
