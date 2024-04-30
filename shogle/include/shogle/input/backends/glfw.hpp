#pragma once

#include <shogle/render/backends/glfw.hpp>

namespace ntf::input {

class glfw : public Singleton<glfw> {
public:
  enum key {
    W = GLFW_KEY_W,
    A = GLFW_KEY_A, 
    S = GLFW_KEY_S,
    D = GLFW_KEY_D,
    J = GLFW_KEY_J,
    K = GLFW_KEY_K,
    L = GLFW_KEY_L,
    Q = GLFW_KEY_Q,
    E = GLFW_KEY_E,
    LEFT = GLFW_KEY_LEFT,
    RIGHT = GLFW_KEY_RIGHT,
    UP = GLFW_KEY_UP,
    DOWN = GLFW_KEY_DOWN,
    LSHIFT = GLFW_KEY_LEFT_SHIFT,
    LCTRL = GLFW_KEY_LEFT_CONTROL,
    TAB = GLFW_KEY_TAB,
    SPACE = GLFW_KEY_SPACE,
    ESCAPE = GLFW_KEY_ESCAPE
  };

  enum key_action {
    PRESS = GLFW_PRESS,
    RELEASE = GLFW_RELEASE
  };
public:
  using keycallback_t = GLFWkeyfun;
  using cursorcallback_t = GLFWcursorposfun;

public:
  glfw() = default;

public:
  void set_key_callback(keycallback_t cb);
  void set_cursor_callback(cursorcallback_t cb);

public:
  inline bool is_pressed(key key) {
    return glfwGetKey(_win(), key) == GLFW_PRESS;
  }
  
private:
  static GLFWwindow* _win(void) { return render::glfw::instance()._win; }
};

} // namespace ntf
