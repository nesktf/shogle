#pragma once

#include "../context.hpp"

namespace ntf {

using win_error = ::ntf::error<void>;

template<typename T>
using win_expected = ::ntf::expected<T, win_error>;

template<typename... Ts>
using win_ctx_params = std::variant<weak_cref<Ts>...>;

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

class renderer_window {
private:
  struct callback_handler_t;
  friend callback_handler_t;
  
  template<typename Signature>
  using fun_t = std::function<Signature>;

public:
  using viewport_fun = fun_t<void(renderer_window&, extent2d)>;
  using key_fun      = fun_t<void(renderer_window&, win_key_data)>;
  using cursorp_fun  = fun_t<void(renderer_window&, dvec2)>;
  using cursore_fun  = fun_t<void(renderer_window&, bool)>;
  using scroll_fun   = fun_t<void(renderer_window&, dvec2)>;
  using mouse_fun    = fun_t<void(renderer_window&, win_button_data)>;
  using char_fun     = fun_t<void(renderer_window&, uint32)>;

private:
  renderer_window(r_window handle, r_api ctx_api) noexcept;

public:
  [[nodiscard]] static win_expected<renderer_window> create(const win_params& params);

public:
  template<typename F>
  requires(std::invocable<F, renderer_window&, extent2d>)
  renderer_window& set_viewport_callback(F&& fun) {
    _callbacks.viewport = std::forward<F>(fun);
    return *this;
  }

  template<typename F>
  requires(std::invocable<F, renderer_window&, win_key_data>)
  renderer_window& set_key_press_callback(F&& fun) {
    _callbacks.keypress = std::forward<F>(fun);
    return *this;
  }

  template<typename F>
  requires(std::invocable<F, renderer_window&, win_button_data>)
  renderer_window& set_button_press_callback(F&& fun) {
    _callbacks.buttpress = std::forward<F>(fun);
    return *this;
  }

  template<typename F>
  requires(std::invocable<F, renderer_window&, dvec2>)
  renderer_window& set_cursor_pos_callback(F&& fun) {
    _callbacks.cursorpos = std::forward<F>(fun);
    return *this;
  }

  template<typename F>
  requires(std::invocable<F, renderer_window&, bool>)
  renderer_window& set_cursor_enter_callback(F&& fun) {
    _callbacks.cursorenter = std::forward<F>(fun);
    return *this;
  }

  template<typename F>
  requires(std::invocable<F, renderer_window&, dvec2>)
  renderer_window& set_scroll_callback(F&& fun) {
    _callbacks.scroll = std::forward<F>(fun);
    return *this;
  }

  template<typename F>
  requires(std::invocable<F, renderer_window&, uint32>)
  renderer_window& set_char_input_callback(F&& fun) {
    _callbacks.char_input = std::forward<F>(fun);
    return *this;
  }

  void title(const std::string& title);
  void close();
  void poll_events();
  void set_mouse_state(win_mouse_state state);

public:
  r_window handle() const { return _handle; }
  r_api renderer() const { return _ctx_api; }

  bool should_close() const;
  win_action poll_key(win_key key) const;
  win_action poll_button(win_button button) const;
  extent2d win_size() const;
  extent2d fb_size() const;

private:
  void _destroy();

private:
  r_window _handle;
  r_api _ctx_api;
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
  NTF_DECLARE_MOVE_ONLY(renderer_window);
};

template<typename F>
concept delta_time_func = std::invocable<F, float64>; // f(dt) -> void

template<typename F>
concept fixed_render_func = std::invocable<F, float64, float64>; // f(dt, alpha) -> void

template<typename F>
concept fixed_update_func = std::invocable<F, uint32>; // f(ups) -> void

template<typename T>
concept has_render_member = requires(T t) {
  { t.on_render(float64{}) } -> std::convertible_to<void>;
};

template<typename T>
concept has_fixed_render_member = requires(T t) {
  { t.on_render(float64{}, float64{}) } -> std::convertible_to<void>;
};

template<typename T>
concept has_fixed_update_member = requires(T t) {
  { t.on_fixed_update(uint32{}) } -> std::convertible_to<void>;
};

template<typename T>
concept fixed_loop_object = 
  (fixed_render_func<T> && fixed_update_func<T>) ||
  (has_fixed_render_member<T> && has_fixed_update_member<T>);

template<typename T>
concept nonfixed_loop_object = delta_time_func<T> || has_render_member<T>;


template<nonfixed_loop_object LoopObj>
void shogle_render_loop(renderer_window& window, r_context_view ctx, LoopObj&& obj) {
  using namespace std::literals;

  using clock = std::chrono::steady_clock;
  using duration = clock::duration;
  using time_point = std::chrono::time_point<clock, duration>;

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Main loop started");

  time_point last_time = clock::now();
  while (!window.should_close()) {
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
    last_time = start_time;

    float64 dt {std::chrono::duration<float64>{elapsed_time}/1s};

    window.poll_events();

    ctx.start_frame();
    if constexpr (has_render_member<LoopObj>) {
      obj.on_render(dt);
    } else {
      obj(dt);
    }
    ctx.end_frame();
  }
  ctx.device_wait();

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Main loop exit");
}

template<fixed_loop_object LoopObj>
void shogle_render_loop(renderer_window& window, r_context_view ctx, const uint32& ups, LoopObj&& obj) {
  using namespace std::literals;

  auto fixed_elapsed_time =
    std::chrono::duration<float64>{std::chrono::microseconds{1000000/ups}};

  using clock = std::chrono::steady_clock;
  using duration = decltype(clock::duration{} + fixed_elapsed_time);
  using time_point = std::chrono::time_point<clock, duration>;

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Fixed main loop started at {} ups", ups);

  time_point last_time = clock::now();
  duration lag = 0s;
  while (!window.should_close()) {
    fixed_elapsed_time = std::chrono::duration<float64>{std::chrono::microseconds{1000000/ups}};
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
    last_time = start_time;
    lag += elapsed_time;

    float64 dt {std::chrono::duration<float64>{elapsed_time}/1s};
    float64 alpha {std::chrono::duration<float64>{lag}/fixed_elapsed_time};

    window.poll_events();

    ctx.start_frame();

    while (lag >= fixed_elapsed_time) {
      if constexpr (has_fixed_update_member<LoopObj>) {
        obj.on_fixed_update(ups);
      } else {
        obj(ups);
      }
      lag -= fixed_elapsed_time;
    }

    if constexpr (has_fixed_render_member<LoopObj>) {
      obj.on_render(dt, alpha);
    } else {
      obj(dt, alpha);
    }

    ctx.end_frame();
  }
  ctx.device_wait();

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Main loop exit");
}

template<fixed_render_func RFunc, fixed_update_func UFunc>
void shogle_render_loop(renderer_window& window, r_context_view ctx, const uint32& ups,
                        RFunc&& render, UFunc&& fixed_update) {
  using namespace std::literals;

  auto fixed_elapsed_time =
    std::chrono::duration<float64>{std::chrono::microseconds{1000000/ups}};

  using clock = std::chrono::steady_clock;
  using duration = decltype(clock::duration{} + fixed_elapsed_time);
  using time_point = std::chrono::time_point<clock, duration>;

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Fixed main loop started at {} ups", ups);

  time_point last_time = clock::now();
  duration lag = 0s;
  while (!window.should_close()) {
    fixed_elapsed_time = std::chrono::duration<float64>{std::chrono::microseconds{1000000/ups}};
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
    last_time = start_time;
    lag += elapsed_time;

    float64 dt {std::chrono::duration<float64>{elapsed_time}/1s};
    float64 alpha {std::chrono::duration<float64>{lag}/fixed_elapsed_time};

    window.poll_events();

    ctx.start_frame();

    while (lag >= fixed_elapsed_time) {
      fixed_update(ups);
      lag -= fixed_elapsed_time;
    }

    render(dt, alpha);

    ctx.end_frame();
  }
  ctx.device_wait();

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Main loop exit");
}

} // namespace ntf
