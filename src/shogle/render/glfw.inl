#define SHOGLE_RENDER_GLFW_INL
#include <shogle/render/glfw.hpp>
#undef SHOGLE_RENDER_GLFW_INL

namespace ntf {

inline glfw::glfw_lib::~glfw_lib() noexcept {
  glfwTerminate();
}


[[nodiscard]] inline auto glfw::init() -> glfw_lib {
  glfwInit();
  return glfw_lib{};
}

template<typename Renderer>
void glfw::set_current_context(const window<Renderer>& window) {
  glfwMakeContextCurrent(window._handle);
}

inline void glfw::set_swap_interval(uint interval) {
  glfwSwapInterval(static_cast<int>(interval));
}

inline void glfw::poll_events() {
  glfwPollEvents();
}


template<typename Renderer>
glfw::window<Renderer>::window(ivec2 sz, std::string_view title) {
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, Renderer::VER_MAJOR);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, Renderer::VER_MINOR);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHintString(GLFW_X11_CLASS_NAME, title.data());

  GLFWwindow* win = glfwCreateWindow(sz.x, sz.y, title.data(), NULL, NULL);

  if (!win) {
    ntf::log::error("[SHOGLE][ntf::glfw::window] Failed to create window");
    return;
  }

  glfwMakeContextCurrent(win);

  if (!Renderer::init(glfwGetProcAddress)) {
    glfwDestroyWindow(win);
    ntf::log::error("[SHOGLE][ntf::glfw::window] Failed to initialize renderer");
    return;
  }
  SHOGLE_INTERNAL_LOG_FMT(verbose,
    "[SHOGLE][ntf::glfw::window] Renderer initialized ({})", Renderer::name_str());

  glfwSetWindowUserPointer(win, this);

  glfwSetFramebufferSizeCallback(win, fb_callback);
  glfwSetKeyCallback(win, key_callback);
  glfwSetCursorPosCallback(win, cursor_callback);
  glfwSetScrollCallback(win, scroll_callback);

  Renderer::set_viewport(sz);
  SHOGLE_INTERNAL_LOG_FMT(verbose,
    "[SHOGLE][ntf::glfw::window] Created (w: {}, h: {}, title: {})", sz.x, sz.y, title);
  _handle = win;
}

template<typename Renderer>
glfw::window<Renderer>::~window() noexcept {
  if (_handle) {
    Renderer::destroy();
    glfwDestroyWindow(_handle);
  }
}

template<typename Renderer>
glfw::window<Renderer>::window(window&& w) noexcept :
  _handle(std::move(w._handle)),
  _viewport_event(std::move(w._viewport_event)), _key_event(std::move(w._key_event)),
  _cursor_event(std::move(w._cursor_event)), _scroll_event(std::move(w._scroll_event)) { w._handle = nullptr; }

template<typename Renderer>
auto glfw::window<Renderer>::operator=(window&& w) noexcept -> window& {
  if (_handle) {
    Renderer::destroy();
    glfwDestroyWindow(_handle);
  }

  _handle = std::move(w._handle);
  _viewport_event = std::move(w._viewport_event);
  _key_event = std::move(w._key_event);
  _cursor_event = std::move(w._key_event);
  _scroll_event = std::move(w._scroll_event);

  w._handle = nullptr;

  glfwSetWindowUserPointer(_handle, this);

  return *this;
}

template<typename Renderer>
void glfw::window<Renderer>::fb_callback(GLFWwindow* win, int w, int h) {
  auto* window = static_cast<glfw::window<Renderer>*>(glfwGetWindowUserPointer(win));
  if (window->_viewport_event) {
    window->_viewport_event(static_cast<size_t>(w), static_cast<size_t>(h));
  }
}

template<typename Renderer>
void glfw::window<Renderer>::key_callback(GLFWwindow* win, int code, int scan, int state, int mod) {
  auto* window = static_cast<glfw::window<Renderer>*>(glfwGetWindowUserPointer(win));
  if (window->_key_event) {
    window->_key_event(
      static_cast<keycode>(code),
      static_cast<scancode>(scan),
      static_cast<keystate>(state),
      static_cast<keymod>(mod)
    );
  }
}

template<typename Renderer>
void glfw::window<Renderer>::cursor_callback(GLFWwindow* win, double xpos, double ypos) {
  auto* window = static_cast<glfw::window<Renderer>*>(glfwGetWindowUserPointer(win));
  if (window->_cursor_event) {
    window->_cursor_event(xpos, ypos);
  }
}

template<typename Renderer>
void glfw::window<Renderer>::scroll_callback(GLFWwindow* win, double xoff, double yoff) {
  auto* window = static_cast<glfw::window<Renderer>*>(glfwGetWindowUserPointer(win));
  if (window->_scroll_event) {
    window->_scroll_event(xoff, yoff);
  }
}

template<typename Renderer>
void glfw::window<Renderer>::set_viewport_event(viewport_event event) {
  _viewport_event = std::move(event);
}

template<typename Renderer>
void glfw::window<Renderer>::set_key_event(key_event event) {
  _key_event = std::move(event);
}

template<typename Renderer>
void glfw::window<Renderer>::set_cursor_event(cursor_event event) {
  _cursor_event = std::move(event);
}

template<typename Renderer>
void glfw::window<Renderer>::set_scroll_event(scroll_event event) {
  _scroll_event = std::move(event);
}

template<typename Renderer>
void glfw::window<Renderer>::set_title(std::string_view title) {
  glfwSetWindowTitle(_handle, title.data());
}

template<typename Renderer>
void glfw::window<Renderer>::close() {
  glfwSetWindowShouldClose(_handle, 1);
}

template<typename Renderer>
void glfw::window<Renderer>::poll_events() {
  glfw::poll_events();
}

template<typename Renderer>
void glfw::window<Renderer>::swap_buffers() {
  glfwSwapBuffers(_handle);
}

template<typename Renderer>
bool glfw::window<Renderer>::should_close() const {
  return glfwWindowShouldClose(_handle);
}

template<typename Renderer>
bool glfw::window<Renderer>::poll_key(keycode key, keystate state) const {
  return glfwGetKey(_handle, static_cast<int>(key)) == static_cast<int>(state);
}

template<typename Renderer>
ivec2 glfw::window<Renderer>::size() const {
  int w, h;
  glfwGetWindowSize(_handle, &w, &h);
  return ivec2{w, h};
}

template<typename Renderer>
void glfw::window<Renderer>::imgui_init(imgui_impl) {
  imgui_impl::init(_handle, true);
}

} // namespace ntf
