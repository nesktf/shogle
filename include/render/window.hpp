#pragma once

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "log.hpp"

namespace ntf::shogle {

class Window {
public:
  Window(size_t w, size_t h, const char* w_title);
  ~Window();

  Window(Window&&) = default;
  Window& operator=(Window&&) = default;

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

public:
  inline void close(void) {
    glfwSetWindowShouldClose(win, true);
    Log::verbose("[Window] Set window close");
  }

  inline void swap_buffers(void) {
    glfwSwapBuffers(win);
  }

  inline bool should_close(void) {
    return glfwWindowShouldClose(win);
  }

  inline void set_fb_callback(GLFWframebuffersizefun cb, void* usr_ptr = nullptr) {
    glfwSetWindowUserPointer(win, usr_ptr);
    glfwSetFramebufferSizeCallback(win, cb);
    Log::verbose("[Window] Framebuffer callback set");
  }

  inline void set_key_callback(GLFWkeyfun cb) {
    glfwSetKeyCallback(win, cb);
    Log::verbose("[Window] Key callback set");
  }

  inline void set_title(const char* w_title) {
    glfwSetWindowTitle(win, w_title);
    Log::verbose("[Window] Title set: {}", w_title);
  }

  inline void poll_events(void) {
    glfwPollEvents();
  }
  
  inline double get_time(void) {
    return glfwGetTime();
  }
  
  inline bool is_button_pressed(int key) {
    return glfwGetKey(win, key) == GLFW_PRESS;
  }

public:
  static std::unique_ptr<Window> create(size_t w, size_t h, const char* w_title = "shogle") {
    return std::make_unique<Window>(w, h, w_title);
  }

public:
  inline size_t width(void) { return w_width; }
  inline size_t height(void) { return w_height; }
  inline float ratio() { return (float)w_width/(float)w_height; }

private:
  GLFWwindow* win;
  size_t w_width;
  size_t w_height;
};

} // namespace ntf::shogle
