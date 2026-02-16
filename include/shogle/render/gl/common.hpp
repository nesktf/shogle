#pragma once

#include <shogle/render/common.hpp>

#include <shogle/util/expected.hpp>
#include <shogle/util/ptr.hpp>

// TODO: Do something to avoid polluting the root namespace with this?
#include <KHR/khrplatform.h>

#include <string>

#define SHOGLE_GL_LOG(level_, fmt_, ...) SHOGLE_RENDER_LOG(level_, "OpenGL", fmt_, __VA_ARGS__)

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

template<typename T>
concept gl_provider_type = requires(T prov, const char* name) {
  { prov.gl_get_proc(name) } -> std::same_as<void*>;
  { prov.surface_extent() } -> std::same_as<extent2d>;
  requires noexcept(prov.gl_get_proc(name));
  requires noexcept(prov.surface_extent());
};

struct gl_version {
  u32 major;
  u32 minor;
};

class gl_surface_provider {
public:
  using PFN_glGetProcAddress = void* (*)(void* user, const char* name) noexcept;

private:
  struct vtbl_t {
    void* (*gl_get_proc)(void* user, const char* name) noexcept;
    extent2d (*surface_extent)(void* user) noexcept;
  };

  template<typename T>
  static constexpr vtbl_t vtbl_for{
    .gl_get_proc = +[](void* user, const char* name) noexcept -> void* {
      return (*static_cast<T*>(user)).gl_get_proc(name);
    },
    .surface_extent =
      +[](void* user) noexcept -> extent2d { return (*static_cast<T*>(user)).surface_extent(); },
  };

public:
  template<gl_provider_type T>
  gl_surface_provider(T& provider) noexcept :
      _provider(std::addressof(provider)), _vtbl(&vtbl_for<T>) {}

public:
  void* proc_address(const char* name) const noexcept {
    return _vtbl->gl_get_proc(_provider, name);
  }

  extent2d surface_extent() const noexcept { return _vtbl->surface_extent(_provider); }

public:
  void* get_ptr() const noexcept { return _provider; }

  PFN_glGetProcAddress get_proc_func() const noexcept { return _vtbl->gl_get_proc; }

private:
  void* _provider;
  const vtbl_t* _vtbl;
};

namespace impl {

::shogle::mem::scratch_arena& gl_get_scratch_arena(gl_context& gl);
gl_private& gl_get_private(gl_context& gl);

} // namespace impl

constexpr gldefs::GLhandle GL_DEFAULT_BINDING = 0;
constexpr gldefs::GLhandle GL_NULL_HANDLE = std::numeric_limits<gldefs::GLhandle>::max();

std::string_view gl_error_string(gldefs::GLenum err) noexcept;

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
using gl_sv_expect = expected<T, gl_sv_error>;

template<typename T>
using gl_s_expect = expected<T, gl_s_error>;

class gl_error : public std::exception {
public:
  gl_error() noexcept : _err(0) {}

  gl_error(gldefs::GLenum err) noexcept : _err(err) {}

public:
  const char* what() const noexcept override { return gl_error_string(_err).data(); }

  gldefs::GLenum code() const noexcept { return _err; }

  std::string_view code_str() const noexcept { return gl_error_string(_err); }

private:
  gldefs::GLenum _err;
};

template<typename T>
using gl_expect = expected<T, gl_error>;

template<typename T>
struct gl_deleter;

template<typename T>
class gl_scoped_resource : public gl_deleter<T> {
public:
  gl_scoped_resource(gl_context& gl, T& obj) noexcept :
      gl_deleter<T>(gl), _obj(std::addressof(obj)), _count(1u) {}

  gl_scoped_resource(gl_context& gl, T* obj, u32 count) noexcept :
      gl_deleter<T>(gl), _obj(obj), _count(count) {}

  gl_scoped_resource(gl_context& gl, span<T> objs) noexcept :
      gl_deleter<T>(gl), _obj(objs.data()), _count(objs.size()) {}

  gl_scoped_resource(const gl_scoped_resource&) = delete;
  gl_scoped_resource(gl_scoped_resource&&) = delete;

  ~gl_scoped_resource() noexcept { destroy(); }

public:
  void disengage() noexcept { _obj = nullptr; }

  void rebind(T& obj) noexcept {
    _obj = std::addressof(obj);
    _count = 1;
  }

  void rebind(T* obj, u32 count) noexcept {
    _obj = std::addressof(obj);
    _count = count;
  }

  void rebind(span<T> objs) noexcept {
    _obj = objs.data();
    _count = objs.size();
  }

  void destroy() noexcept {
    if (_obj) {
      auto& self = static_cast<gl_deleter<T>&>(*this);
      _count > 1 ? std::invoke(self, _obj, _count) : std::invoke(self, *_obj);
      disengage();
    }
  }

public:
  bool is_active() const noexcept { return _obj != nullptr; }

public:
  gl_scoped_resource& operator=(const gl_scoped_resource&) = delete;
  gl_scoped_resource& operator=(gl_scoped_resource&&) = delete;

  operator bool() const noexcept { return is_active(); }

private:
  T* _obj;
  size_t _count;
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
