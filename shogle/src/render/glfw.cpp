#include <shogle/render/glfw.hpp>

#include <shogle/core/log.hpp>
#include <shogle/core/error.hpp>

#define GL_MAJOR 3
#define GL_MINOR 3

namespace ntf::render {

glfw::window glfw::create_window(size_t width, size_t height, std::string title) {
  window win {};

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHintString(GLFW_X11_CLASS_NAME, "shogle");

  if (!(win.glfw_win = glfwCreateWindow(width, height, title.c_str(), NULL, NULL))) {
    glfwTerminate();
    throw ntf::error{"[GLFW window] Failed to create window"};
  }

  glfwMakeContextCurrent(win.glfw_win);
  glfwSwapInterval(1); // Vsync

  try {
    gl::init((GLADloadproc)glfwGetProcAddress);
  } catch (const ntf::error& err) {
    glfw::destroy_window(win);
    throw;
  }
  gl::set_viewport(width, height);

  return win;
}

void glfw::destroy_window(window& win) {
  glfwDestroyWindow(win.glfw_win);
  glfwTerminate();
  win.glfw_win = nullptr;
}

void glfw::set_viewport_callback(window& win, viewportfun_t cb) {
  glfwSetFramebufferSizeCallback(win.glfw_win, cb);
}

void glfw::set_title(window& win, std::string title) {
  win.win_title = title;
  glfwSetWindowTitle(win.glfw_win, win.win_title.c_str());
}

void glfw::set_close(window& win, bool flag) {
  glfwSetWindowShouldClose(win.glfw_win, flag);
}

vec2 glfw::window_size(window& win) {
  int w, h;
  glfwGetWindowSize(win.glfw_win, &w, &h);
  return {static_cast<float>(w), static_cast<float>(h)};
}

void glfw::set_key_callback(window& win, keycallback_t cb) {
  glfwSetKeyCallback(win.glfw_win, cb);
}

void glfw::set_cursor_callback(window& win, cursorcallback_t cb) {
  glfwSetCursorPosCallback(win.glfw_win, cb);
}

void glfw::set_user_ptr(window& win, void* ptr) {
  glfwSetWindowUserPointer(win.glfw_win, ptr);
}

void* glfw::get_user_ptr(GLFWwindow* win) {
  return glfwGetWindowUserPointer(win);
}



} // namespace ntf::render
