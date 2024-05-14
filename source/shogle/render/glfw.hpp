#pragma once

#include <shogle/core/singleton.hpp>
#include <shogle/core/types.hpp>

#include <GLFW/glfw3.h>

#include <string>

namespace ntf::shogle::glfw {

using viewportfun = GLFWframebuffersizefun;
using keyfun = GLFWkeyfun;
using cursorfun = GLFWcursorposfun;

struct window {
  GLFWwindow* handle {nullptr};
  std::string title {};
};

window create_window(vec2sz sz, std::string title);
void destroy_window(window& win);
void close_window(window& win);

void set_viewport_callback(window& win, viewportfun fun);
void set_key_callback(window& win, keyfun fun);
void set_cursor_callback(window& win, cursorfun fun);

void set_user_ptr(window& win, void* ptr);
void* get_user_ptr(GLFWwindow* handle);

void set_title(window& win, std::string title);
vec2sz window_size(window& win);

inline void poll_events(window&) {
  glfwPollEvents();
}
inline void swap_buffers(window& win) {
  glfwSwapBuffers(win.handle);
}

inline bool is_window_open(window& win) {
  return !glfwWindowShouldClose(win.handle);
}

inline double elapsed_time(window&) {
  return glfwGetTime();
}

inline bool is_key_pressed(window& win, int key) {
  return glfwGetKey(win.handle, key) == GLFW_PRESS;
}

} // namespace ntf::shogle::glfw
