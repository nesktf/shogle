#include <shogle/render/gl/render.hpp>
#include <shogle/render/glfw/window.hpp>

#include <shogle/core/error.hpp>
#include <shogle/core/log.hpp>

#define GL_MAJOR 3
#define GL_MINOR 3

namespace ntf::shogle::glfw {

void window::_framebuffer_size_cb(GLFWwindow* win, int w, int h) {
  auto* self = static_cast<window*>(glfwGetWindowUserPointer(win));
  self->viewport_event(*self, static_cast<size_t>(w), static_cast<size_t>(h));
}

void window::_key_event_cb(GLFWwindow* win, int code, int scan, int state, int mod) {
  auto* self = static_cast<window*>(glfwGetWindowUserPointer(win));
  self->key_event(*self, 
    static_cast<keycode>(code), 
    static_cast<scancode>(scan),
    static_cast<keystate>(state), 
    static_cast<keymod>(mod)
  );
}

void window::_cursor_event_cb(GLFWwindow* win, double xpos, double ypos) {
  auto* self = static_cast<window*>(glfwGetWindowUserPointer(win));
  self->cursor_event(*self, xpos, ypos);
}

void window::_scroll_event_cb(GLFWwindow* win, double xoff, double yoff) {
  auto* self = static_cast<window*>(glfwGetWindowUserPointer(win));
  self->scroll_event(*self, xoff, yoff);
}

window::window(size_t w, size_t h, std::string title) :
  _title(title) {
  glfwInit();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHintString(GLFW_X11_CLASS_NAME, "shogle");

  GLFWwindow* win;
  if (!(win = glfwCreateWindow(w, h, title.c_str(), NULL, NULL))) {
    glfwTerminate();
    throw ntf::error{"[GLFW] Failed to create window"};
  }

  glfwMakeContextCurrent(win);
  _handle = win;
  log::verbose("[glfw::window] Window created (w: {}, h: {}, title: {})", w, h, _title);

  glfwSetWindowUserPointer(_handle, this);
  glfwSetFramebufferSizeCallback(_handle, _framebuffer_size_cb);
  glfwSetKeyCallback(_handle, _key_event_cb);
  glfwSetCursorPosCallback(_handle, _cursor_event_cb);
  glfwSetScrollCallback(_handle, _scroll_event_cb);
  log::verbose("[glfw::window] Event callbacks set");

  try {
    gl::init((GLADloadproc)glfwGetProcAddress);
    gl::set_viewport_size(w, h);
  } catch(...) {
    glfwDestroyWindow(win);
    glfwTerminate();
    throw;
  }
}

window::~window() {
  gl::terminate();
  glfwDestroyWindow(_handle);
  glfwTerminate();
  log::verbose("[glfw::window] Window destroyed");
}

void window::close() {
  glfwSetWindowShouldClose(_handle, true);
}

window& window::set_title(std::string title) {
  _title = std::move(title);
  glfwSetWindowTitle(_handle, _title.c_str());
  return *this;
}

vec2sz window::size() const {
  int w, h;
  glfwGetWindowSize(_handle, &w, &h);
  return vec2sz{(size_t)w, (size_t)h};
}

// void set_input_mode(window& win, bool enable) {
//   glfwSetInputMode(win.handle, GLFW_CURSOR, enable ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
// }

} // namespace ntf::shogle::glfw
