#pragma once

#include "../../math/alg.hpp"

#include <GLFW/glfw3.h>

namespace ntf {

enum class glfw_keycode : int {
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

enum class glfw_keystate : int {
  press = GLFW_PRESS,
  release = GLFW_RELEASE,
  repeat = GLFW_REPEAT,
};


using glfw_scancode = int;

enum class glfw_keymod : int {
  mod_shift = GLFW_MOD_SHIFT,
  mod_ctrl = GLFW_MOD_CONTROL,
  mod_alt = GLFW_MOD_ALT,
  mod_super = GLFW_MOD_SUPER,
  mod_capslock = GLFW_MOD_CAPS_LOCK,
  mod_numlock = GLFW_MOD_NUM_LOCK,
};

enum class glfw_mousebutton : int {
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

enum class glfw_profile {
  any = GLFW_OPENGL_ANY_PROFILE,
  compat = GLFW_OPENGL_COMPAT_PROFILE,
  core = GLFW_OPENGL_CORE_PROFILE,
};

struct glfw_hints {
  glfw_profile profile = glfw_profile::core;
  int context_ver_maj = 3;
  int context_ver_min = 3;
  std::string_view x11_class_name = "";
  std::string_view x11_instance_name = "";
};

class glfw_lib {
public:
  using keycode = glfw_keycode;
  using scancode = glfw_scancode;
  using keystate = glfw_keystate;
  using keymod = glfw_keymod;

private:
  glfw_lib(bool succ) noexcept 
    { _inited = succ; }

public:
  ~glfw_lib() noexcept {
    if (_inited) {
      glfwTerminate();
      _inited = false;
    }
  }

public:
  static void set_swap_interval(uint interval) {
    NTF_ASSERT(_inited);
    glfwSwapInterval(static_cast<int>(interval));
  }

  template<is_forwarding<glfw_hints> Hints>
  static void apply_hints(Hints&& hints) {
    NTF_ASSERT(_inited);
    glfwWindowHint(GLFW_OPENGL_PROFILE, static_cast<int>(hints.profile));
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, static_cast<int>(hints.context_ver_maj));
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, static_cast<int>(hints.context_ver_min));
    glfwWindowHintString(GLFW_X11_CLASS_NAME, hints.x11_class_name.data());
    glfwWindowHintString(GLFW_X11_INSTANCE_NAME, hints.x11_instance_name.data());
  }

  static std::string_view error() {
    const char* err;
    if (glfwGetError(&err) == GLFW_NO_ERROR) {
      return std::string_view{};
    }
    return std::string_view{err};
  }

public:
  bool valid() const { return _inited; }
  explicit operator bool() const { return valid(); }

private:
  static inline bool _inited{false};

public:
  NTF_DISABLE_MOVE_COPY(glfw_lib);

private:
  friend glfw_lib glfw_init();
};

[[nodiscard]] inline glfw_lib glfw_init() {
  NTF_ASSERT(!glfw_lib::_inited);
  return glfw_lib{glfwInit() == GLFW_TRUE};
}

} // namespace ntf
