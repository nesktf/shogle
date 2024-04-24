#pragma once

#include "glad/glad.h"

#include <GLFW/glfw3.h>

namespace ntf {

enum InputButtons {
  KEY_W = GLFW_KEY_W,
  KEY_A = GLFW_KEY_A, 
  KEY_S = GLFW_KEY_S,
  KEY_D = GLFW_KEY_D,
  KEY_J = GLFW_KEY_J,
  KEY_K = GLFW_KEY_K,
  KEY_L = GLFW_KEY_L,
  KEY_Q = GLFW_KEY_Q,
  KEY_E = GLFW_KEY_E,
  KEY_LEFT = GLFW_KEY_LEFT,
  KEY_RIGHT = GLFW_KEY_RIGHT,
  KEY_UP = GLFW_KEY_UP,
  KEY_DOWN = GLFW_KEY_DOWN,
  KEY_LSHIFT = GLFW_KEY_LEFT_SHIFT,
  KEY_LCTRL = GLFW_KEY_LEFT_CONTROL,
  KEY_TAB = GLFW_KEY_TAB,
  KEY_SPACE = GLFW_KEY_SPACE,
  KEY_ESCAPE = GLFW_KEY_ESCAPE
};

enum InputAction {
  KEY_PRESS = GLFW_PRESS,
  KEY_RELEASE = GLFW_RELEASE
};

class GLWindow {
public:
  using fbcallback_t = GLFWframebuffersizefun;
  using keycallback_t = GLFWkeyfun;
  using cursorposcallback_t = GLFWcursorposfun;

public:
  GLWindow(size_t w, size_t h, const char* w_title);
  ~GLWindow();

  GLWindow(GLWindow&&) = default;
  GLWindow(const GLWindow&) = delete;
  GLWindow& operator=(GLWindow&&) = default;
  GLWindow& operator=(const GLWindow&) = delete;

public:
  void close(void);
  void swap_buffers(void);

  void set_fb_callback(fbcallback_t cb);
  void set_key_callback(keycallback_t cb);
  void set_cursorpos_callback(cursorposcallback_t cb);

  void set_title(const char* title);

public:
  inline void poll_events(void) {
    glfwPollEvents();
  }

  inline bool should_close(void) {
    return glfwWindowShouldClose(_glfw_win);
  }
  
  inline double get_time(void) {
    return glfwGetTime();
  }
  
  inline bool is_button_pressed(int key) {
    return glfwGetKey(_glfw_win, key) == GLFW_PRESS;
  }

  inline size_t width(void) { return _win_w; }
  inline size_t height(void) { return _win_h; }
  inline float ratio() { return (float)_win_w/(float)_win_h; }

public:
  bool imgui_demo {true};

private:
  GLFWwindow* _glfw_win;
  size_t _win_w, _win_h;
};

} // namespace ntf
