#pragma once

#include <shogle/core/singleton.hpp>
#include <shogle/core/types.hpp>

#include <shogle/render/gl.hpp>

#include <GLFW/glfw3.h>

#include <string>

namespace ntf::render {

class glfw : public Singleton<glfw> {
public:
  using viewportfun_t = GLFWframebuffersizefun;
  using keycallback_t = GLFWkeyfun;
  using cursorcallback_t = GLFWcursorposfun;

  struct window {
    GLFWwindow* glfw_win {nullptr};
    std::string win_title {};
  };

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
  glfw() = default;

public:
  static window create_window(size_t width, size_t height, std::string title);
  static void destroy_window(window& win);
 
  static void set_viewport_callback(window& win, viewportfun_t cb);
  static void set_key_callback(window& win, keycallback_t cb);
  static void set_cursor_callback(window& win, cursorcallback_t cb);

  static void set_user_ptr(window& win, void* ptr);
  static void* get_user_ptr(GLFWwindow* win);

  static void set_title(window& win, std::string title);
  static void set_close(window& win, bool flag = true);

  static vec2 window_size(window& win);

public:
  static inline void new_frame(window&) {
    glfwPollEvents();
  }

  static inline void end_frame(window& win) {
    glfwSwapBuffers(win.glfw_win);
  }

  static inline bool is_window_open(window& win) {
    return !glfwWindowShouldClose(win.glfw_win);
  }

  static inline double elapsed_time(window&) {
    return glfwGetTime();
  }

  static inline bool is_key_pressed(window& win, key key) {
    return glfwGetKey(win.glfw_win, key) == GLFW_PRESS;
  }
};


} // namespace ntf::render
