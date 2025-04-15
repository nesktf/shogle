#include "./window.hpp"

namespace ntf {

namespace {

static std::atomic<uint32> win_count = 0;

template<bool checked>
auto renderer_window_create_impl(
  const win_params& params
) -> std::conditional_t<checked, win_expected<renderer_window>, renderer_window>
{
#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
  if (win_count.load() == 0) {
    if (!glfwInit()) {
      const char* err;
      glfwGetError(&err);
      if constexpr (checked) {
        SHOGLE_LOG(error, "[ntf::renderer_window] Failed to initialize GLFW: {}", err);
        return unexpected{win_error::format({"Failed to initialize GLFW: {}"}, err)};
      } else {
        NTF_ASSERT(false, "[ntf::renderer_window] Failed to initialize GLFW: {}", err);
      }
    }
    SHOGLE_LOG(verbose, "[ntf::renderer_window][OTHER] GLFW initialized");
  }

  renderer_api ctx_api;
  params.ctx_params | ::ntf::overload {
    [&](weak_ref<win_gl_params> gl_params) {
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_params->ver_major);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_params->ver_minor);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      ctx_api = renderer_api::opengl;
    },
    [&](weak_ref<win_vk_params>) {
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      ctx_api = renderer_api::vulkan;
    }
  };

  if (params.x11_class_name) {
    glfwWindowHintString(GLFW_X11_CLASS_NAME, params.x11_class_name);
  }

  if (params.x11_instance_name) {
    glfwWindowHintString(GLFW_X11_INSTANCE_NAME, params.x11_instance_name);
  }

  const char* title = params.title ? params.title : "window - ShOGLE";

  win_handle handle = glfwCreateWindow(params.width, params.height, title, nullptr, nullptr);
  if (!handle) {
    if (win_count.load() == 0) {
      glfwTerminate();
      SHOGLE_LOG(verbose, "[ntf::r_window] GLFW terminated");
    }

    const char* err;
    glfwGetError(&err);
    if constexpr (checked) {
      SHOGLE_LOG(error, "[ntf::r_window] Failed to create GLFW window: {}", err);
      return unexpected{win_error::format({"Failed to create GLFW window: {}"}, err)};
    } else {
      NTF_ASSERT(false, "[ntf::r_window] Failed to create GLFW window: {}", err);
    }
  }
  ++win_count;

  SHOGLE_LOG(debug, "[ntf::r_window] GLFW window created");
  if constexpr (checked) {
    return win_expected<renderer_window>{::ntf::in_place, handle, ctx_api};
  } else {
    return renderer_window{handle, ctx_api};
  }
#endif
}

} // namespace

win_expected<renderer_window> renderer_window::create(const win_params& params) {
  return renderer_window_create_impl<true>(params);
}

renderer_window renderer_window::create(unchecked_t, const win_params& params) {
  return renderer_window_create_impl<false>(params);
}

renderer_window::renderer_window(win_handle handle, renderer_api ctx_api) noexcept :
  _handle{handle}, _ctx_api{ctx_api}
{ 
  glfwSetWindowUserPointer(_handle, this);
  _bind_callbacks();
}

renderer_window::~renderer_window() noexcept { _reset(true); }

void renderer_window::_reset(bool destroy) {
#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
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

renderer_window::renderer_window(renderer_window&& other) noexcept :
  _handle{std::move(other._handle)},
  _ctx_api{std::move(other._ctx_api)},
  _vp_event{std::move(other._vp_event)},
  _key_event{std::move(other._key_event)},
  _cur_event{std::move(other._cur_event)},
  _scl_event{std::move(other._scl_event)}
{
  other._reset(false);
  glfwSetWindowUserPointer(_handle, this);
  // _bind_callbacks();
}

renderer_window& renderer_window::operator=(renderer_window&& other) noexcept {
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
  glfwSetWindowUserPointer(_handle, this);
  // _bind_callbacks();

  return *this;
}

void renderer_window::_bind_callbacks() {
#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
  NTF_ASSERT(_handle);
  glfwSetFramebufferSizeCallback(_handle, renderer_window::fb_callback);
  glfwSetKeyCallback(_handle, renderer_window::key_callback);
  glfwSetCursorPosCallback(_handle, renderer_window::cursor_callback);
  glfwSetScrollCallback(_handle, renderer_window::scroll_callback);
#endif
}

void renderer_window::close() {
#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
  glfwSetWindowShouldClose(_handle, 1);
#endif
}

void renderer_window::title(const std::string& title) {
#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
  glfwSetWindowTitle(_handle, title.c_str());
#endif
}

bool renderer_window::should_close() const {
#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
  NTF_ASSERT(_handle);
  return glfwWindowShouldClose(_handle);
#endif
}

bool renderer_window::poll_key(win_keycode key, win_keystate state) const {
#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
  NTF_ASSERT(_handle);
  return glfwGetKey(_handle, static_cast<int>(key)) == static_cast<int>(state);
#endif
}

uvec2 renderer_window::win_size() const {
#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
  NTF_ASSERT(_handle);
  int w, h;
  glfwGetWindowSize(_handle, &w, &h);
  return uvec2{static_cast<uint32>(w), static_cast<uint32>(h)};
#endif
}

uvec2 renderer_window::fb_size() const {
#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
  NTF_ASSERT(_handle);
  int w, h;
  glfwGetFramebufferSize(_handle, &w, &h);
  return uvec2{static_cast<uint32>(w), static_cast<uint32>(h)};
#endif
}

void renderer_window::poll_events() {
#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
  glfwPollEvents();
#endif
}

#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
void renderer_window::fb_callback(GLFWwindow* handle, int w, int h) {
  auto* ptr = glfwGetWindowUserPointer(handle);
  NTF_ASSERT(ptr);
  auto& win = *static_cast<renderer_window*>(ptr);
  if (win._vp_event) {
    win._vp_event(win, static_cast<uint32>(w), static_cast<uint32>(h));
  }
}

void renderer_window::key_callback(GLFWwindow* handle, int code, int scan, int state, int mod) {
  auto* ptr = glfwGetWindowUserPointer(handle);
  NTF_ASSERT(ptr);
  auto& win = *static_cast<renderer_window*>(ptr);
  if (win._key_event) {
    win._key_event(
      win,
      static_cast<win_keycode>(code),
      static_cast<win_scancode>(scan),
      static_cast<win_keystate>(state),
      static_cast<win_keymod>(mod)
    );
  }
}

void renderer_window::cursor_callback(GLFWwindow* handle, double xpos, double ypos) {
  auto* ptr = glfwGetWindowUserPointer(handle);
  NTF_ASSERT(ptr);
  auto& win = *static_cast<renderer_window*>(ptr);
  if (win._cur_event) {
    win._cur_event(win, static_cast<float64>(xpos), static_cast<float64>(ypos));
  }
}

void renderer_window::scroll_callback(GLFWwindow* handle, double xoff, double yoff) {
  auto* ptr = glfwGetWindowUserPointer(handle);
  NTF_ASSERT(ptr);
  auto& win = *static_cast<renderer_window*>(ptr);
  if (win._scl_event) {
    win._scl_event(win, static_cast<float64>(xoff), static_cast<float64>(yoff));
  }
}
#endif

} // namespace ntf
