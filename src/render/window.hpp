#pragma once

#include <glad/glad.h>

#include "./render.hpp"
#include "./keys.hpp"
#include "../stl/function.hpp"
#include "../stl/optional.hpp"

#include <variant>

namespace ntf {

template<typename... Ts>
using r_win_ctx_params = std::variant<weak_ref<Ts>...>;

struct r_win_gl_params {
  uint32 ver_major;
  uint32 ver_minor;
};

struct r_win_vk_params {};

struct r_win_params {
  uint32 width;
  uint32 height;

  const char* title;
  const char* x11_class_name;
  const char* x11_instance_name;

  r_win_ctx_params<r_win_gl_params, r_win_vk_params> ctx_params;
};

#if SHOGLE_USE_GLFW
using win_handle_t = GLFWwindow*;
#endif

class r_window {
public:
  template<typename Signature>
  using callback_type = std::function<Signature>;

  using viewport_fun = callback_type<void(r_window&,uint32,uint32)>;
  using key_fun = callback_type<void(r_window&,keycode,scancode,keystate,keymod)>;
  using cursor_fun = callback_type<void(r_window&,float64,float64)>;
  using scroll_fun = callback_type<void(r_window&,float64,float64)>;

#if SHOGLE_USE_GLFW
private:
  static void fb_callback(win_handle_t handle, int w, int h);
  static void key_callback(win_handle_t handle, int code, int scan, int state, int mod);
  static void cursor_callback(win_handle_t handle, double xpos, double ypos);
  static void scroll_callback(win_handle_t handle, double xoff, double yoff);
#endif

public:
  r_window(win_handle_t handle, r_api ctx_api);

public:
  [[nodiscard]] static r_expected<r_window> create(const r_win_params& params);
  [[nodiscard]] static r_window create(unchecked_t, const r_win_params& params);

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
  win_handle_t handle() const { return _handle; }
  r_api ctx_api() const { return _ctx_api; }

  bool should_close() const;
  bool poll_key(keycode key, keystate state) const;
  uvec2 win_size() const;
  uvec2 fb_size() const;

private:
  void _bind_callbacks();
  void _reset(bool destroy);

public:
  static bool vk_surface(win_handle_t handle,
                         VkInstance instance, VkSurfaceKHR* surface,
                         const VkAllocationCallbacks* alloc);
  static void gl_set_current(win_handle_t handle);
  static void gl_set_swap_interval(uint32 interval);
  static void gl_swap_buffers(win_handle_t handle);
  static GLADloadproc gl_load_proc();

private:
  win_handle_t _handle;
  r_api _ctx_api;

  viewport_fun _vp_event;
  key_fun _key_event;
  cursor_fun _cur_event;
  scroll_fun _scl_event;

public:
  NTF_DECLARE_MOVE_ONLY(r_window);
};

} // namespace ntf
