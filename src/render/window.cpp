#include "./internal/platform.hpp"
#include "./window.hpp"

namespace ntf {

#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
static GLFWwindow* win_cast(r_window win) { return reinterpret_cast<GLFWwindow*>(win); }
static r_window win_cast(GLFWwindow* win) { return reinterpret_cast<r_window>(win); }
#endif

struct renderer_window::callback_handler_t {
#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
  static void fb_callback(GLFWwindow* handle, int w, int h) {
    auto* ptr = glfwGetWindowUserPointer(handle);
    NTF_ASSERT(ptr);
    auto& win = *static_cast<renderer_window*>(ptr);
    if (win._vp_event) {
      win._vp_event(win, {w, h});
    }
  }

  static void key_callback(GLFWwindow* handle, int code, int scan, int state, int mod) {
    auto* ptr = glfwGetWindowUserPointer(handle);
    NTF_ASSERT(ptr);
    auto& win = *static_cast<renderer_window*>(ptr);
    if (win._key_event) {
      win._key_event(win,{
        static_cast<win_keycode>(code),
        static_cast<win_scancode>(scan),
        static_cast<win_keystate>(state),
        static_cast<win_keymod>(mod)
      });
    }
  }

  static void cursor_callback(GLFWwindow* handle, double xpos, double ypos) {
    auto* ptr = glfwGetWindowUserPointer(handle);
    NTF_ASSERT(ptr);
    auto& win = *static_cast<renderer_window*>(ptr);
    if (win._cur_event) {
      win._cur_event(win, {xpos, ypos});
    }
  }

  static void scroll_callback(GLFWwindow* handle, double xoff, double yoff) {
    auto* ptr = glfwGetWindowUserPointer(handle);
    NTF_ASSERT(ptr);
    auto& win = *static_cast<renderer_window*>(ptr);
    if (win._scl_event) {
      win._scl_event(win, {xoff, yoff});
    }
  }
#endif
};

static std::atomic<uint32> win_count = 0;

win_expected<renderer_window> renderer_window::create(const win_params& params) {
#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
  if (win_count.load() == 0) {
    if (!glfwInit()) {
      const char* err;
      glfwGetError(&err);
      SHOGLE_LOG(error, "[ntf::renderer_window] Failed to initialize GLFW: {}", err);
      return unexpected{win_error::format({"Failed to initialize GLFW: {}"}, err)};
    }
    SHOGLE_LOG(verbose, "[ntf::renderer_window][OTHER] GLFW initialized");
  }

  r_api ctx_api;
  params.ctx_params | ::ntf::overload {
    [&](weak_ref<win_gl_params> gl_params) {
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_params->ver_major);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_params->ver_minor);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      ctx_api = r_api::opengl;
    },
    [&](weak_ref<win_vk_params>) {
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      ctx_api = r_api::vulkan;
    }
  };

  if (params.x11_class_name) {
    glfwWindowHintString(GLFW_X11_CLASS_NAME, params.x11_class_name);
  }

  if (params.x11_instance_name) {
    glfwWindowHintString(GLFW_X11_INSTANCE_NAME, params.x11_instance_name);
  }

  const char* title = params.title ? params.title : "window - ShOGLE";

  GLFWwindow* handle = glfwCreateWindow(params.width, params.height, title, nullptr, nullptr);
  if (!handle) {
    if (win_count.load() == 0) {
      glfwTerminate();
      SHOGLE_LOG(verbose, "[ntf::r_window] GLFW terminated");
    }

    const char* err;
    glfwGetError(&err);
    SHOGLE_LOG(error, "[ntf::r_window] Failed to create GLFW window: {}", err);
    return unexpected{win_error::format({"Failed to create GLFW window: {}"}, err)};
  }
  ++win_count;

  return renderer_window{win_cast(handle), ctx_api};
#endif
}

renderer_window::renderer_window(r_window handle, r_api ctx_api) noexcept :
  _handle{handle}, _ctx_api{ctx_api}
{ 
  GLFWwindow* win = win_cast(_handle);
  glfwSetFramebufferSizeCallback(win, callback_handler_t::fb_callback);
  glfwSetKeyCallback(win, callback_handler_t::key_callback);
  glfwSetCursorPosCallback(win, callback_handler_t::cursor_callback);
  glfwSetScrollCallback(win, callback_handler_t::scroll_callback);
  glfwSetWindowUserPointer(win, this);
}

renderer_window::~renderer_window() noexcept { _destroy(); }

void renderer_window::_destroy() {
#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
  if (!_handle) {
    return;
  }
  glfwDestroyWindow(win_cast(_handle));
  SHOGLE_LOG(debug, "[ntf::r_window][CONSTRUCT]");
  if (--win_count == 0) {
    glfwTerminate();
    SHOGLE_LOG(verbose, "[ntf::r_window][OTHER] GLFW terminated");
  }
#endif
}

renderer_window::renderer_window(renderer_window&& other) noexcept :
  _handle{std::move(other._handle)}, _ctx_api{std::move(other._ctx_api)},
  _callbacks{std::move(other._callbacks)}
{
  other._handle = nullptr;
  glfwSetWindowUserPointer(win_cast(_handle), this);
}

renderer_window& renderer_window::operator=(renderer_window&& other) noexcept {
  if (std::addressof(other) == this) {
    return *this;
  }

  _destroy();

  _handle = std::move(other._handle);
  _ctx_api = std::move(other._ctx_api);
  _callbacks = std::move(other._callbacks);

  other._handle = nullptr;
  glfwSetWindowUserPointer(win_cast(_handle), this);

  return *this;
}

void renderer_window::close() {
#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
  glfwSetWindowShouldClose(win_cast(_handle), 1);
#endif
}

void renderer_window::title(const std::string& title) {
#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
  glfwSetWindowTitle(win_cast(_handle), title.c_str());
#endif
}

bool renderer_window::should_close() const {
#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
  NTF_ASSERT(_handle);
  return glfwWindowShouldClose(win_cast(_handle));
#endif
}

win_keystate renderer_window::poll_key(win_keycode key) const {
#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
  NTF_ASSERT(_handle);
  return static_cast<win_keystate>(glfwGetKey(win_cast(_handle), static_cast<int>(key)));
#endif
}

uvec2 renderer_window::win_size() const {
#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
  NTF_ASSERT(_handle);
  int w, h;
  glfwGetWindowSize(win_cast(_handle), &w, &h);
  return uvec2{static_cast<uint32>(w), static_cast<uint32>(h)};
#endif
}

uvec2 renderer_window::fb_size() const {
#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
  NTF_ASSERT(_handle);
  int w, h;
  glfwGetFramebufferSize(win_cast(_handle), &w, &h);
  return uvec2{static_cast<uint32>(w), static_cast<uint32>(h)};
#endif
}

void renderer_window::poll_events() {
#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
  glfwPollEvents();
#endif
}

} // namespace ntf
