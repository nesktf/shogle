#include "./window.hpp"

namespace ntf {

r_window::~r_window() noexcept { reset(); }

bool r_window::init_context(ctx_params_t ctx_params) {
#if SHOGLE_USE_GLFW
  if (!glfwInit()) {
    SHOGLE_LOG(error, "[ntf::r_window] Failed to initialize GLFW");
    return false;
  }
  switch (ctx_params.api) {
    case r_api::opengl: {
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, ctx_params.gl_maj);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, ctx_params.gl_min);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      break;
    }
    default: {
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }
  }
  if (!_init_params.x11_class_name.empty()) {
    glfwWindowHintString(GLFW_X11_CLASS_NAME, _init_params.x11_class_name.data());
  }

  if (!_init_params.x11_instance_name.empty()) {
    glfwWindowHintString(GLFW_X11_INSTANCE_NAME, _init_params.x11_instance_name.data());
  }

  GLFWwindow* win = glfwCreateWindow(
    _init_params.width, _init_params.height, _init_params.title.data(), nullptr, nullptr
  );
  if (!win) {
    glfwTerminate();
    SHOGLE_LOG(error, "[ntf::r_window] Failed to create window");
    return false;
  }
  if (ctx_params.api == r_api::opengl) {
    glfwMakeContextCurrent(win);
    glfwSwapInterval(ctx_params.swap_interval.value_or(0));
  }

  glfwSetWindowUserPointer(win, this);

  glfwSetFramebufferSizeCallback(win, r_window::fb_callback);
  glfwSetKeyCallback(win, r_window::key_callback);
  glfwSetCursorPosCallback(win, r_window::cursor_callback);
  glfwSetScrollCallback(win, r_window::scroll_callback);
  _handle = win;
#endif

  return true;
}

void r_window::reset() {
  if (!valid()) {
    return;
  }

  glfwDestroyWindow(_handle);
  glfwTerminate();
  _handle = nullptr;
}

void r_window::fb_callback(GLFWwindow* handle, int w, int h) {
  auto& win = *reinterpret_cast<r_window*>(glfwGetWindowUserPointer(handle));
  if (win._viewport_event) {
    win._viewport_event(static_cast<uint32>(w), static_cast<uint32>(h));
  }
}

void r_window::key_callback(GLFWwindow* handle, int code, int scan, int state, int mod) {
  auto& win = *reinterpret_cast<r_window*>(glfwGetWindowUserPointer(handle));
  if (win._key_event) {
    win._key_event(
      static_cast<keycode>(code),
      static_cast<scancode>(scan),
      static_cast<keystate>(state),
      static_cast<keymod>(mod)
    );
  }
}

void r_window::cursor_callback(GLFWwindow* handle, double xpos, double ypos) {
  auto& win = *reinterpret_cast<r_window*>(glfwGetWindowUserPointer(handle));
  if (win._cursor_event) {
    win._cursor_event(static_cast<float64>(xpos), static_cast<float64>(ypos));
  }
}

void r_window::scroll_callback(GLFWwindow* handle, double xoff, double yoff) {
  auto& win = *reinterpret_cast<r_window*>(glfwGetWindowUserPointer(handle));
  if (win._scroll_event) {
    win._scroll_event(static_cast<float64>(xoff), static_cast<float64>(yoff));
  }
}

void r_window::viewport_event(viewport_fun callback) {
  _viewport_event = std::move(callback);
}

void r_window::key_event(key_fun callback) {
  _key_event = std::move(callback);
}

void r_window::cursor_event(cursor_fun callback) {
  _cursor_event = std::move(callback);
}

void r_window::scroll_event(scroll_fun callback) {
  _scroll_event = std::move(callback);
}

void r_window::close() {
  glfwSetWindowShouldClose(_handle, 1);
}

bool r_window::should_close() const {
  NTF_ASSERT(_handle);
  return glfwWindowShouldClose(_handle);
}

bool r_window::poll_key(keycode key, keystate state) const {
  NTF_ASSERT(_handle);
  return glfwGetKey(_handle, static_cast<int>(key)) == static_cast<int>(state);
}

uvec2 r_window::win_size() const {
  NTF_ASSERT(_handle);
  int w, h;
  glfwGetWindowSize(_handle, &w, &h);
  return uvec2{static_cast<uint32>(w), static_cast<uint32>(h)};
}

uvec2 r_window::fb_size() const {
  NTF_ASSERT(_handle);
  int w, h;
  glfwGetFramebufferSize(_handle, &w, &h);
  return uvec2{static_cast<uint32>(w), static_cast<uint32>(h)};
}

void r_window::poll_events() {
  glfwPollEvents();
}

void r_window::swap_buffers() {
  glfwSwapBuffers(_handle);
}

} // namespace ntf
