#include "./opengl/context.hpp"
#include "./window.hpp"

namespace ntf {

#ifdef SHOGLE_USE_GLFW
r_window::~r_window() noexcept {
  if (!valid()) {
    return;
  }

  if (_ctx_api == r_api::opengl) {
    auto* ctx = reinterpret_cast<gl_context*>(_ctx);
    ctx->destroy();
  }
#ifdef SHOGLE_ENABLE_IMGUI
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
#endif
  glfwDestroyWindow(_handle);
  glfwTerminate();
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
#ifdef SHOGLE_ENABLE_IMGUI
  ImGui_ImplGlfw_NewFrame(); // TODO: Move this to the context new frame?
#endif
}
#endif

} // namespace ntf
