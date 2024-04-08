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
  KEY_L = GLFW_KEY_L
};

enum InputAction {
  KEY_PRESS = GLFW_PRESS,
  KEY_RELEASE = GLFW_RELEASE
};

class GLWindow {
public:
  using fbcallback_t = GLFWframebuffersizefun;
  using keycallback_t = GLFWkeyfun;

public:
  GLWindow(size_t w, size_t h, const char* w_title);
  ~GLWindow();

  GLWindow(GLWindow&&) = default;
  GLWindow& operator=(GLWindow&&) = default;

  GLWindow(const GLWindow&) = delete;
  GLWindow& operator=(const GLWindow&) = delete;

public:
  void close(void);
  void swap_buffers(void);

  void set_fb_callback(fbcallback_t cb);
  void set_key_callback(keycallback_t cb);

  void set_title(const char* title);

public:
  inline void poll_events(void) {
    glfwPollEvents();
  }

  inline bool should_close(void) {
    return glfwWindowShouldClose(win);
  }
  
  inline double get_time(void) {
    return glfwGetTime();
  }
  
  inline bool is_button_pressed(int key) {
    return glfwGetKey(win, key) == GLFW_PRESS;
  }

  inline size_t width(void) { return w_width; }
  inline size_t height(void) { return w_height; }
  inline float ratio() { return (float)w_width/(float)w_height; }

private:
  GLFWwindow* win;
  size_t w_width, w_height;
};

} // namespace ntf
