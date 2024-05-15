#include <shogle/render/gl/render.hpp>
#include <shogle/render/api/glfw.hpp>

#include <shogle/core/error.hpp>

#define GL_MAJOR 3
#define GL_MINOR 3

namespace ntf::shogle::glfw {

window create_window(vec2sz sz, std::string title) {
  GLFWwindow* win;
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHintString(GLFW_X11_CLASS_NAME, "shogle");

  if (!(win = glfwCreateWindow(sz.w, sz.h, title.c_str(), NULL, NULL))) {
    glfwTerminate();
    throw ntf::error{"[GLFW] Failed to create window"};
  }

  glfwMakeContextCurrent(win);

  try {
    gl::init((GLADloadproc)glfwGetProcAddress);
  } catch(...) {
    glfwDestroyWindow(win);
    glfwTerminate();
    throw;
  }

  gl::set_viewport_size(sz);

  return window{win, std::move(title)};
}

void destroy_window(window& win) {
  if (!win.handle) return;
  gl::terminate();
  glfwDestroyWindow(win.handle);
  glfwTerminate();
  win.handle = nullptr;
}

void close_window(window& win) {
  glfwSetWindowShouldClose(win.handle, true);
}

void set_viewport_callback(window& win, viewportfun fun) {
  glfwSetFramebufferSizeCallback(win.handle, fun);
}

void set_key_callback(window& win, keyfun fun) {
  glfwSetKeyCallback(win.handle, fun);
}

void set_cursor_callback(window& win, cursorfun fun) {
  glfwSetCursorPosCallback(win.handle, fun);
}

void set_user_ptr(window& win, void* ptr) {
  glfwSetWindowUserPointer(win.handle, ptr);
}

void* get_user_ptr(GLFWwindow* handle) {
  return glfwGetWindowUserPointer(handle);
}

void set_title(window& win, std::string title) {
  win.title = std::move(title);
  glfwSetWindowTitle(win.handle, win.title.c_str());
}

vec2sz window_size(window& win) {
  int w, h;
  glfwGetWindowSize(win.handle, &w, &h);
  return vec2sz{(size_t)w, (size_t)h};
}

} // namespace ntf::glfw
