#define SHOGLE_RENDER_GLFW_WINDOW_INL
#include "./window.hpp"
#undef SHOGLE_RENDER_GLFW_WINDOW_INL

namespace ntf {

template<typename RenderContext>
glfw_window<RenderContext>::glfw_window(std::size_t w, std::size_t h, std::string_view title) {
  GLFWwindow* win = glfwCreateWindow(w, h, title.data(), nullptr, nullptr);

  if (!win) {
    ntf::logger::error("[SHOGLE][ntf::glfw_window] Failed to create window");
    return;
  }

  glfwMakeContextCurrent(win);

  if (!_render_ctx.init(glfwGetProcAddress)) {
    glfwDestroyWindow(win);
    ntf::logger::error("[SHOGLE][ntf::glfw_window] Failed to initialize renderer");
    return;
  }
  _render_ctx.set_viewport(w, h);
  SHOGLE_LOG(verbose, "[ntf::glfw_window] Render context initialized \"{} {} {}\"",
             _render_ctx.name(), _render_ctx.version(), _render_ctx.vendor());

  glfwSetWindowUserPointer(win, this);

  glfwSetFramebufferSizeCallback(win, fb_callback);
  glfwSetKeyCallback(win, key_callback);
  glfwSetCursorPosCallback(win, cursor_callback);
  glfwSetScrollCallback(win, scroll_callback);

#ifdef SHOGLE_ENABLE_IMGUI
  if constexpr (imgui_enabled) {
    constexpr bool enable_callbacks = true;
    if (!imgui_init<imgui_impl>(_handle, enable_callbacks, "#version 130")) {
      _render_ctx.destroy();
      glfwDestroyWindow(_handle);
      SHOGLE_LOG(error, "[ntf::glfw_window] Failed to initialize imgui");
      return;
    }
  }
  SHOGLE_LOG(verbose, "[ntf::glfw_window] Imgui initialized");
#endif

  _handle = win;

  SHOGLE_LOG(verbose, "[ntf::glfw_window] Window created (w: {}, h: {}, title: {})", w, h, title);
}

template<typename RenderContext>
glfw_window<RenderContext>::~glfw_window() noexcept {
  if (_handle) {
#ifdef SHOGLE_ENABLE_IMGUI
    if constexpr (imgui_enabled) {
      imgui_destroy<imgui_impl>();
    }
#endif
    _render_ctx.destroy();
    glfwDestroyWindow(_handle);
  }
}

template<typename RenderContext>
void glfw_window<RenderContext>::fb_callback(GLFWwindow* win, int w, int h) {
  auto* window = static_cast<glfw_window<RenderContext>*>(glfwGetWindowUserPointer(win));
  NTF_ASSERT(window, "Attempted to dereference null glfw window");

  if (window->_viewport_event) {
    window->_viewport_event(static_cast<size_t>(w), static_cast<size_t>(h));
  }
}

template<typename RenderContext>
void glfw_window<RenderContext>::key_callback(GLFWwindow* win, int code, int scan,
                                              int state, int mod) {
  auto* window = static_cast<glfw_window<RenderContext>*>(glfwGetWindowUserPointer(win));
  NTF_ASSERT(window, "Attempted to dereference null glfw window");

  if (window->_key_event) {
    window->_key_event(
      static_cast<glfw_keycode>(code),
      static_cast<glfw_scancode>(scan),
      static_cast<glfw_keystate>(state),
      static_cast<glfw_keymod>(mod)
    );
  }
}

template<typename RenderContext>
void glfw_window<RenderContext>::cursor_callback(GLFWwindow* win, double xpos, double ypos) {
  auto* window = static_cast<glfw_window<RenderContext>*>(glfwGetWindowUserPointer(win));
  NTF_ASSERT(window, "Attempted to dereference null glfw window");

  if (window->_cursor_event) {
    window->_cursor_event(xpos, ypos);
  }
}


template<typename RenderContext>
void glfw_window<RenderContext>::scroll_callback(GLFWwindow* win, double xoff, double yoff) {
  auto* window = static_cast<glfw_window<RenderContext>*>(glfwGetWindowUserPointer(win));
  NTF_ASSERT(window, "Attempted to dereference null glfw window");

  if (window->_scroll_event) {
    window->_scroll_event(xoff, yoff);
  }
}


template<typename RenderContext>
void glfw_window<RenderContext>::set_viewport_event(viewport_event event) {
  NTF_ASSERT(_handle, "Invalid glfw_window");
  _viewport_event = std::move(event);
}


template<typename RenderContext>
void glfw_window<RenderContext>::set_key_event(key_event event) {
  NTF_ASSERT(_handle, "Invalid glfw_window");
  _key_event = std::move(event);
}

template<typename RenderContext>
void glfw_window<RenderContext>::set_cursor_event(cursor_event event) {
  NTF_ASSERT(_handle, "Invalid glfw_window");
  _cursor_event = std::move(event);
}

template<typename RenderContext>
void glfw_window<RenderContext>::set_scroll_event(scroll_event event) {
  NTF_ASSERT(_handle, "Invalid glfw_window");
  _scroll_event = std::move(event);
}

template<typename RenderContext>
void glfw_window<RenderContext>::set_title(std::string_view title) {
  NTF_ASSERT(_handle, "Invalid glfw_window");
  glfwSetWindowTitle(_handle, title.data());
}

template<typename RenderContext>
void glfw_window<RenderContext>::close() {
  NTF_ASSERT(_handle, "Invalid glfw_window");
  glfwSetWindowShouldClose(_handle, 1);
}

template<typename RenderContext>
void glfw_window<RenderContext>::poll_events() {
  NTF_ASSERT(_handle, "Invalid glfw_window");
  glfwPollEvents();
  if constexpr (imgui_enabled) {
    imgui_start_frame<imgui_impl>();
  }
}

template<typename RenderContext>
void glfw_window<RenderContext>::swap_buffers() {
  NTF_ASSERT(_handle, "Invalid glfw_window");
  if constexpr (imgui_enabled) {
    imgui_end_frame<imgui_impl>();
  }
  glfwSwapBuffers(_handle);
}

template<typename RenderContext>
bool glfw_window<RenderContext>::should_close() const {
  NTF_ASSERT(_handle, "Invalid glfw_window");
  return glfwWindowShouldClose(_handle);
}

template<typename RenderContext>
bool glfw_window<RenderContext>::poll_key(glfw_keycode key, glfw_keystate state) const {
  NTF_ASSERT(_handle, "Invalid glfw_window");
  return glfwGetKey(_handle, static_cast<int>(key)) == static_cast<int>(state);
}

template<typename RenderContext>
ivec2 glfw_window<RenderContext>::size() const {
  NTF_ASSERT(_handle, "Invalid glfw_window");
  int w, h;
  glfwGetWindowSize(_handle, &w, &h);
  return ivec2{w, h};
}

} // namespace ntf
