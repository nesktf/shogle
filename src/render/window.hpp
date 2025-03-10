#pragma once

#include "./types.hpp"

#include "../stl/function.hpp"
#include "../stl/optional.hpp"

namespace ntf {

#ifdef SHOGLE_USE_GLFW
enum class keycode : int {
  key_unknown = GLFW_KEY_UNKNOWN,
  key_space = GLFW_KEY_SPACE,
  key_apostrophe = GLFW_KEY_APOSTROPHE,
  key_comma = GLFW_KEY_COMMA,
  key_minus = GLFW_KEY_MINUS,
  key_period = GLFW_KEY_PERIOD,
  key_slash = GLFW_KEY_SLASH,
  key_0 = GLFW_KEY_0,
  key_1 = GLFW_KEY_1,
  key_2 = GLFW_KEY_2,
  key_3 = GLFW_KEY_3,
  key_4 = GLFW_KEY_4,
  key_5 = GLFW_KEY_5,
  key_7 = GLFW_KEY_7,
  key_8 = GLFW_KEY_8,
  key_9 = GLFW_KEY_9,
  key_semicolon = GLFW_KEY_SEMICOLON,
  key_equal = GLFW_KEY_EQUAL,
  key_a = GLFW_KEY_A,
  key_b = GLFW_KEY_B,
  key_c = GLFW_KEY_C,
  key_d = GLFW_KEY_D,
  key_e = GLFW_KEY_E,
  key_f = GLFW_KEY_F,
  key_g = GLFW_KEY_G,
  key_h = GLFW_KEY_H,
  key_i = GLFW_KEY_I,
  key_j = GLFW_KEY_J,
  key_k = GLFW_KEY_K,
  key_l = GLFW_KEY_L,
  key_m = GLFW_KEY_M,
  key_n = GLFW_KEY_N,
  key_o = GLFW_KEY_O,
  key_p = GLFW_KEY_P,
  key_q = GLFW_KEY_Q,
  key_r = GLFW_KEY_R,
  key_s = GLFW_KEY_S,
  key_t = GLFW_KEY_T,
  key_u = GLFW_KEY_U,
  key_v = GLFW_KEY_V,
  key_w = GLFW_KEY_W,
  key_x = GLFW_KEY_X,
  key_y = GLFW_KEY_Y,
  key_z = GLFW_KEY_Z,
  key_lbracket = GLFW_KEY_LEFT_BRACKET,
  key_backslash = GLFW_KEY_BACKSLASH,
  key_rbracket = GLFW_KEY_RIGHT_BRACKET,
  key_grave = GLFW_KEY_GRAVE_ACCENT,
  key_w1 = GLFW_KEY_WORLD_1,
  key_w2 = GLFW_KEY_WORLD_2,
  key_escape = GLFW_KEY_ESCAPE,
  key_enter = GLFW_KEY_ENTER,
  key_tab = GLFW_KEY_TAB,
  key_backspace = GLFW_KEY_BACKSPACE,
  key_insert = GLFW_KEY_INSERT,
  key_delete = GLFW_KEY_DELETE,
  key_right = GLFW_KEY_RIGHT,
  key_left = GLFW_KEY_LEFT,
  key_down = GLFW_KEY_DOWN,
  key_up = GLFW_KEY_UP,
  key_pgup = GLFW_KEY_PAGE_UP,
  key_pgdown = GLFW_KEY_PAGE_DOWN,
  key_home = GLFW_KEY_HOME,
  key_end = GLFW_KEY_END,
  key_capslock = GLFW_KEY_CAPS_LOCK,
  key_scrolllock = GLFW_KEY_SCROLL_LOCK,
  key_numlock = GLFW_KEY_NUM_LOCK,
  key_prntscr = GLFW_KEY_PRINT_SCREEN,
  key_pause = GLFW_KEY_PAUSE,
  key_f1 = GLFW_KEY_F1,
  key_f2 = GLFW_KEY_F2,
  key_f3 = GLFW_KEY_F3,
  key_f4 = GLFW_KEY_F4,
  key_f5 = GLFW_KEY_F5,
  key_f6 = GLFW_KEY_F6,
  key_f7 = GLFW_KEY_F7,
  key_f8 = GLFW_KEY_F8,
  key_f9 = GLFW_KEY_F9,
  key_f10 = GLFW_KEY_F10,
  key_f11 = GLFW_KEY_F11,
  key_f12 = GLFW_KEY_F12,
  key_f13 = GLFW_KEY_F13,
  key_f14 = GLFW_KEY_F14,
  key_f15 = GLFW_KEY_F15,
  key_f16 = GLFW_KEY_F16,
  key_f17 = GLFW_KEY_F17,
  key_f18 = GLFW_KEY_F18,
  key_f19 = GLFW_KEY_F19,
  key_f20 = GLFW_KEY_F20,
  key_f21 = GLFW_KEY_F21,
  key_f22 = GLFW_KEY_F22,
  key_f23 = GLFW_KEY_F23,
  key_f24 = GLFW_KEY_F24,
  key_f25 = GLFW_KEY_F25,
  key_kp_0 = GLFW_KEY_KP_0,
  key_kp_1 = GLFW_KEY_KP_1,
  key_kp_2 = GLFW_KEY_KP_2,
  key_kp_3 = GLFW_KEY_KP_3,
  key_kp_4 = GLFW_KEY_KP_4,
  key_kp_5 = GLFW_KEY_KP_5,
  key_kp_6 = GLFW_KEY_KP_6,
  key_kp_7 = GLFW_KEY_KP_7,
  key_kp_8 = GLFW_KEY_KP_8,
  key_kp_9 = GLFW_KEY_KP_9,
  key_kp_dec = GLFW_KEY_KP_DECIMAL,
  key_kp_div = GLFW_KEY_KP_DIVIDE,
  key_kp_mul = GLFW_KEY_KP_MULTIPLY,
  key_kp_sub = GLFW_KEY_KP_SUBTRACT,
  key_kp_add = GLFW_KEY_KP_ADD,
  key_kp_enter = GLFW_KEY_KP_ENTER,
  key_kp_equal = GLFW_KEY_KP_EQUAL,
  key_lshift = GLFW_KEY_LEFT_SHIFT,
  key_lctrl = GLFW_KEY_LEFT_CONTROL,
  key_lalt = GLFW_KEY_LEFT_ALT,
  key_lsuper = GLFW_KEY_LEFT_SUPER,
  key_rshift = GLFW_KEY_RIGHT_SHIFT,
  key_rctrl = GLFW_KEY_RIGHT_CONTROL,
  key_ralt = GLFW_KEY_RIGHT_ALT,
  key_rsuper = GLFW_KEY_RIGHT_SUPER,
  key_menu = GLFW_KEY_MENU,
};

enum class keystate : int {
  press = GLFW_PRESS,
  release = GLFW_RELEASE,
  repeat = GLFW_REPEAT,
};


using scancode = int;

enum class keymod : int {
  mod_shift = GLFW_MOD_SHIFT,
  mod_ctrl = GLFW_MOD_CONTROL,
  mod_alt = GLFW_MOD_ALT,
  mod_super = GLFW_MOD_SUPER,
  mod_capslock = GLFW_MOD_CAPS_LOCK,
  mod_numlock = GLFW_MOD_NUM_LOCK,
};

enum class mousebutton : int {
  mouse_1 = GLFW_MOUSE_BUTTON_1,
  mouse_2 = GLFW_MOUSE_BUTTON_2,
  mouse_3 = GLFW_MOUSE_BUTTON_3,
  mouse_4 = GLFW_MOUSE_BUTTON_4,
  mouse_5 = GLFW_MOUSE_BUTTON_5,
  mouse_6 = GLFW_MOUSE_BUTTON_6,
  mouse_7 = GLFW_MOUSE_BUTTON_7,
  mouse_8 = GLFW_MOUSE_BUTTON_8,
  mouse_last = GLFW_MOUSE_BUTTON_LAST,
  mouse_left = GLFW_MOUSE_BUTTON_LEFT,
  mouse_right = GLFW_MOUSE_BUTTON_RIGHT,
  mouse_middle = GLFW_MOUSE_BUTTON_MIDDLE,
};

// TODO: Add gamepad things
#endif

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
