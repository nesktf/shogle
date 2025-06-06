#pragma once

#include "./context.hpp"

namespace ntf::render {

using win_error = render_error;

template<typename T>
using win_expect = ::ntf::expected<T, win_error>;

struct win_gl_params {
  uint32 width;
  uint32 height;

  const char* title;
  const char* x11_class_name;
  const char* x11_instance_name;

  uint32 ver_major;
  uint32 ver_minor;
};

enum class win_key : int16 { // Follows GLFW key values
  unknown = -1,
  space = 32, apostrophe = 39,

  comma = 44, minus, period, slash,
  k0, k1, k2, k3, k4, k5, k6, k7, k8, k9,

  semicolon = 59, equal = 61,

  a = 65, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,
  lbracket, backslash, rbracket,

  grave = 96,

  world1 = 161, world2, // ?

  escape = 256, enter, tab, backspace, insert, del, right, left, down, up,
  pgup, pgdown, home, end,

  capslock = 280, scrolllock, numlock, printscr, pause,

  f1 = 290, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,
  f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25,

  kp0 = 320, kp1, kp2, kp3, kp4, kp5, kp6, kp7, kp8, kp9,
  kpdec, kpdiv, kpmul, kpsub, kpadd, kpenter, kpequal,

  lshift = 340, lctrl, lalt, lsuper, rshift, rctrl, ralt, rsuper, menu,
};

using win_keyscancode = int32;

enum class win_button : uint8 { // Follows GLFW button values
  m1 = 0, m2, m3, m4, m5, m6, m7, m8,
};

enum class win_action : uint8 { // Follows GLFW action values
  release = 0,
  press,
  repeat,
};

enum class win_keymod : uint8 { // Follows GLFW mod values
  none     = 0x00,
  shift    = 0x01,
  ctrl     = 0x02,
  alt      = 0x04,
  super    = 0x08,
  capslock = 0x10,
  numlock  = 0x20,
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(win_keymod);

enum class win_mouse_state : int8 {
  normal = 0,
  hidden,
  disabled,
};

struct win_key_data {
  win_key key;
  win_keyscancode scancode;
  win_action action;
  win_keymod mod;
};

struct win_button_data {
  win_button button;
  win_action action;
  win_keymod mod;
};

class window {
private:
  struct callback_handler_t;
  friend callback_handler_t;
  
  template<typename Signature>
  using fun_t = std::function<Signature>;

public:
  using viewport_fun = fun_t<void(window&, extent2d)>;
  using key_fun      = fun_t<void(window&, win_key_data)>;
  using cursorp_fun  = fun_t<void(window&, dvec2)>;
  using cursore_fun  = fun_t<void(window&, bool)>;
  using scroll_fun   = fun_t<void(window&, dvec2)>;
  using mouse_fun    = fun_t<void(window&, win_button_data)>;
  using char_fun     = fun_t<void(window&, uint32)>;

public:
  window(window_t handle, context_api ctx_api) noexcept;

public:
  [[nodiscard]] static win_expect<window> create(const win_gl_params& params);

public:
  template<typename F>
  requires(std::invocable<F, window&, extent2d>)
  window& set_viewport_callback(F&& fun) {
    _callbacks.viewport = std::forward<F>(fun);
    return *this;
  }

  template<typename F>
  requires(std::invocable<F, window&, win_key_data>)
  window& set_key_press_callback(F&& fun) {
    _callbacks.keypress = std::forward<F>(fun);
    return *this;
  }

  template<typename F>
  requires(std::invocable<F, window&, win_button_data>)
  window& set_button_press_callback(F&& fun) {
    _callbacks.buttpress = std::forward<F>(fun);
    return *this;
  }

  template<typename F>
  requires(std::invocable<F, window&, dvec2>)
  window& set_cursor_pos_callback(F&& fun) {
    _callbacks.cursorpos = std::forward<F>(fun);
    return *this;
  }

  template<typename F>
  requires(std::invocable<F, window&, bool>)
  window& set_cursor_enter_callback(F&& fun) {
    _callbacks.cursorenter = std::forward<F>(fun);
    return *this;
  }

  template<typename F>
  requires(std::invocable<F, window&, dvec2>)
  window& set_scroll_callback(F&& fun) {
    _callbacks.scroll = std::forward<F>(fun);
    return *this;
  }

  template<typename F>
  requires(std::invocable<F, window&, uint32>)
  window& set_char_input_callback(F&& fun) {
    _callbacks.char_input = std::forward<F>(fun);
    return *this;
  }

  void title(const std::string& title);
  void close();
  void poll_events();
  void set_mouse_state(win_mouse_state state);

public:
  window_t get() const { return _handle; }
  context_api renderer() const { return _ctx_api; }

  bool should_close() const;
  win_action poll_key(win_key key) const;
  win_action poll_button(win_button button) const;
  extent2d win_size() const;
  extent2d fb_size() const;

private:
  void _destroy();

private:
  window_t _handle;
  context_api _ctx_api;
  struct {
    viewport_fun viewport;
    key_fun keypress;
    mouse_fun buttpress;
    cursorp_fun cursorpos;
    cursore_fun cursorenter;
    scroll_fun scroll;
    char_fun char_input;
  } _callbacks;

public:
  NTF_DECLARE_MOVE_ONLY(window);
};

} // namespace ntf::render
