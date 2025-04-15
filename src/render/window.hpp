#pragma once

#include "../platform_macros.hpp"

#include "../stl/ptr.hpp"
#include "../stl/function.hpp"
#include "../stl/expected.hpp"

#include "../math/vector.hpp"

namespace ntf {

using win_error = error<void>;

template<typename T>
using win_expected = expected<T, win_error>;

template<typename... Ts>
using win_ctx_params = std::variant<weak_ref<Ts>...>;

struct win_gl_params {
  uint32 ver_major;
  uint32 ver_minor;
};

struct win_vk_params {};

struct win_params {
  uint32 width;
  uint32 height;

  const char* title;
  const char* x11_class_name;
  const char* x11_instance_name;

  win_ctx_params<win_gl_params, win_vk_params> ctx_params;
};

class renderer_window {
private:
  template<typename Signature>
  using fun_t = std::function<Signature>;

public:
  using viewport_fun = fun_t<void(renderer_window&,uint32,uint32)>;
  using key_fun = fun_t<void(renderer_window&,win_keycode,win_scancode,win_keystate,win_keymod)>;
  using cursor_fun = fun_t<void(renderer_window&,float64,float64)>;
  using scroll_fun = fun_t<void(renderer_window&,float64,float64)>;

#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
private:
  static void fb_callback(win_handle handle, int w, int h);
  static void key_callback(win_handle handle, int code, int scan, int state, int mod);
  static void cursor_callback(win_handle handle, double xpos, double ypos);
  static void scroll_callback(win_handle handle, double xoff, double yoff);
#endif

public:
  renderer_window(win_handle handle, renderer_api ctx_api) noexcept;

public:
  [[nodiscard]] static win_expected<renderer_window> create(const win_params& params);
  [[nodiscard]] static renderer_window create(unchecked_t, const win_params& params);

public:
  template<typename F>
  void viewport_event(F&& callback) {
    _vp_event = std::forward<F>(callback);
  }

  template<typename F>
  void key_event(F&& callback) {
    _key_event = std::forward<F>(callback);
  }

  template<typename F>
  void cursor_event(F&& callback) {
    _cur_event = std::forward<F>(callback);
  }

  template<typename F>
  void scroll_event(F&& callback) {
    _scl_event = std::forward<F>(callback);
  }

  void title(const std::string& title);
  void close();
  void poll_events();

public:
  win_handle handle() const { return _handle; }
  renderer_api renderer() const { return _ctx_api; }

  bool should_close() const;
  bool poll_key(win_keycode key, win_keystate state) const;
  extent2d win_size() const;
  extent2d fb_size() const;

private:
  void _bind_callbacks();
  void _reset(bool destroy);

private:
  win_handle _handle;
  renderer_api _ctx_api;

  viewport_fun _vp_event;
  key_fun _key_event;
  cursor_fun _cur_event;
  scroll_fun _scl_event;

public:
  NTF_DECLARE_MOVE_ONLY(renderer_window);
};

} // namespace ntf
