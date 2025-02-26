#include "./window.hpp"

namespace ntf {

static std::atomic<uint32> win_count = 0;

template<bool checked>
auto r_window_create_impl(const r_win_params& params)
    -> std::conditional_t<checked, r_expected<r_window>, r_window>
{
#if SHOGLE_USE_GLFW
  if (win_count.load() == 0) {
    if (!glfwInit()) {
      const char* err;
      glfwGetError(&err);
      if constexpr (checked) {
        SHOGLE_LOG(error, "[ntf::r_window] Failed to initialize GLFW: {}", err);
        return unexpected{r_error::format({"Failed to initialize GLFW: {}"}, err)};
      } else {
        NTF_ASSERT(false, "[ntf::r_window] Failed to initialize GLFW: {}", err);
      }
    }
    SHOGLE_LOG(verbose, "[ntf::r_window][OTHER] GLFW initialized");
  }

  r_api ctx_api;
  params.ctx_params | ::ntf::overload {
    [&](weak_ref<r_win_gl_params> gl_params) {
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_params->ver_major);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_params->ver_minor);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      ctx_api = r_api::opengl;
    },
    [&](weak_ref<r_win_vk_params>) {
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

  const char* title = params.title ? params.title : "window - ShOGLE " SHOGLE_VERSION_STRING;

  win_handle_t handle = glfwCreateWindow(params.width, params.height, title, nullptr, nullptr);
  if (!handle) {
    if (win_count.load() == 0) {
      glfwTerminate();
      SHOGLE_LOG(verbose, "[ntf::r_window] GLFW terminated");
    }

    const char* err;
    glfwGetError(&err);
    if constexpr (checked) {
      SHOGLE_LOG(error, "[ntf::r_window] Failed to create GLFW window: {}", err);
      return unexpected{r_error::format({"Failed to create GLFW window: {}"}, err)};
    } else {
      NTF_ASSERT(false, "[ntf::r_window] Failed to create GLFW window: {}", err);
    }
  }
  ++win_count;

  SHOGLE_LOG(debug, "[ntf::r_window] GLFW window created");
  if constexpr (checked) {
    return r_expected<r_window>{::ntf::in_place, handle, ctx_api};
  } else {
    return r_window{handle, ctx_api};
  }
#endif
}

r_expected<r_window> r_window::create(const r_win_params& params) {
  return r_window_create_impl<true>(params);
}

r_window r_window::create(unchecked_t, const r_win_params& params) {
  return r_window_create_impl<false>(params);
}

r_window::r_window(win_handle_t handle, r_api ctx_api) :
  _handle{handle}, _ctx_api{ctx_api} { _bind_callbacks(); }

r_window::~r_window() noexcept { _reset(true); }

void r_window::_reset(bool destroy) {
#if SHOGLE_USE_GLFW
  if (destroy && _handle) {
    glfwDestroyWindow(_handle);
    SHOGLE_LOG(debug, "[ntf::r_window][CONSTRUCT]");
    if (--win_count == 0) {
      glfwTerminate();
      SHOGLE_LOG(verbose, "[ntf::r_window][OTHER] GLFW terminated");
    }
  } else {
    _handle = nullptr;
  }
#endif
}

r_window::r_window(r_window&& other) noexcept :
  _handle{std::move(other._handle)},
  _ctx_api{std::move(other._ctx_api)},
  _vp_event{std::move(other._vp_event)},
  _key_event{std::move(other._key_event)},
  _cur_event{std::move(other._cur_event)},
  _scl_event{std::move(other._scl_event)} {
  other._reset(false);
  _bind_callbacks();
}

r_window& r_window::operator=(r_window&& other) noexcept {
  if (std::addressof(other) == this) {
    return *this;
  }

  _reset(true);

  _handle = std::move(other._handle);
  _ctx_api = std::move(other._ctx_api);
  _vp_event = std::move(other._vp_event);
  _key_event = std::move(other._key_event);
  _cur_event = std::move(other._cur_event);

  other._reset(false);
  _bind_callbacks();

  return *this;
}

void r_window::_bind_callbacks() {
#if SHOGLE_USE_GLFW
  NTF_ASSERT(_handle);
  glfwSetWindowUserPointer(_handle, this);
  glfwSetFramebufferSizeCallback(_handle, r_window::fb_callback);
  glfwSetKeyCallback(_handle, r_window::key_callback);
  glfwSetCursorPosCallback(_handle, r_window::cursor_callback);
  glfwSetScrollCallback(_handle, r_window::scroll_callback);
#endif
}

void r_window::close() {
#if SHOGLE_USE_GLFW
  glfwSetWindowShouldClose(_handle, 1);
#endif
}

void r_window::title(const std::string& title) {
#ifdef SHOGLE_USE_GLFW
  glfwSetWindowTitle(_handle, title.c_str());
#endif
}

bool r_window::should_close() const {
#if SHOGLE_USE_GLFW
  NTF_ASSERT(_handle);
  return glfwWindowShouldClose(_handle);
#endif
}

bool r_window::poll_key(keycode key, keystate state) const {
#if SHOGLE_USE_GLFW
  NTF_ASSERT(_handle);
  return glfwGetKey(_handle, static_cast<int>(key)) == static_cast<int>(state);
#endif
}

uvec2 r_window::win_size() const {
#if SHOGLE_USE_GLFW
  NTF_ASSERT(_handle);
  int w, h;
  glfwGetWindowSize(_handle, &w, &h);
  return uvec2{static_cast<uint32>(w), static_cast<uint32>(h)};
#endif
}

uvec2 r_window::fb_size() const {
#if SHOGLE_USE_GLFW
  NTF_ASSERT(_handle);
  int w, h;
  glfwGetFramebufferSize(_handle, &w, &h);
  return uvec2{static_cast<uint32>(w), static_cast<uint32>(h)};
#endif
}

void r_window::poll_events() {
#if SHOGLE_USE_GLFW
  glfwPollEvents();
#endif
}

bool r_window::vk_surface(win_handle_t handle,
                          VkInstance instance, VkSurfaceKHR* surface,
                          const VkAllocationCallbacks* alloc) {
#if SHOGLE_USE_GLFW
  return glfwCreateWindowSurface(instance, handle, alloc, surface);
#endif
}

void r_window::gl_set_current(win_handle_t handle) {
#if SHOGLE_USE_GLFW
  glfwMakeContextCurrent(handle);
#endif
}

void r_window::gl_set_swap_interval(uint32 interval) {
#if SHOGLE_USE_GLFW
  glfwSwapInterval(interval);
#endif
}

GLADloadproc r_window::gl_load_proc() {
#if SHOGLE_USE_GLFW
  return reinterpret_cast<GLADloadproc>(glfwGetProcAddress);
#endif
}

void r_window::gl_swap_buffers(win_handle_t handle) {
#if SHOGLE_USE_GLFW
  glfwSwapBuffers(handle);
#endif
}

#if SHOGLE_USE_GLFW
void r_window::fb_callback(GLFWwindow* handle, int w, int h) {
  auto* ptr = glfwGetWindowUserPointer(handle);
  NTF_ASSERT(ptr);
  auto& win = *static_cast<r_window*>(ptr);
  if (win._vp_event) {
    win._vp_event(win, static_cast<uint32>(w), static_cast<uint32>(h));
  }
}

void r_window::key_callback(GLFWwindow* handle, int code, int scan, int state, int mod) {
  auto* ptr = glfwGetWindowUserPointer(handle);
  NTF_ASSERT(ptr);
  auto& win = *static_cast<r_window*>(ptr);
  if (win._key_event) {
    win._key_event(
      win,
      static_cast<keycode>(code),
      static_cast<scancode>(scan),
      static_cast<keystate>(state),
      static_cast<keymod>(mod)
    );
  }
}

void r_window::cursor_callback(GLFWwindow* handle, double xpos, double ypos) {
  auto* ptr = glfwGetWindowUserPointer(handle);
  NTF_ASSERT(ptr);
  auto& win = *static_cast<r_window*>(ptr);
  if (win._cur_event) {
    win._cur_event(win, static_cast<float64>(xpos), static_cast<float64>(ypos));
  }
}

void r_window::scroll_callback(GLFWwindow* handle, double xoff, double yoff) {
  auto* ptr = glfwGetWindowUserPointer(handle);
  NTF_ASSERT(ptr);
  auto& win = *static_cast<r_window*>(ptr);
  if (win._scl_event) {
    win._scl_event(win, static_cast<float64>(xoff), static_cast<float64>(yoff));
  }
}
#endif

} // namespace ntf
