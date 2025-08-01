#include "./internal/platform.hpp"

#include <shogle/render/window.hpp>

#include <ntfstl/utility.hpp>
#include <atomic>

namespace shogle {

static constexpr u32 round_pow2(u32 x) {
  x--;
  x |= x >> (1<<0);
  x |= x >> (1<<1);
  x |= x >> (1<<2);
  x |= x >> (1<<3);
  x |= x >> (1<<4);
  x++;
  return x;
}

#if defined(SHOGLE_ENABLE_GLFW) && SHOGLE_ENABLE_GLFW
static GLFWwindow* win_cast(window_t win) { return reinterpret_cast<GLFWwindow*>(win); }
static window_t win_cast(GLFWwindow* win) { return reinterpret_cast<window_t>(win); }
#endif

struct window::callback_handler_t {
#if defined(SHOGLE_ENABLE_GLFW) && SHOGLE_ENABLE_GLFW
  static void fb_size_callback(GLFWwindow* handle, int w, int h) {
    auto* ptr = glfwGetWindowUserPointer(handle);
    NTF_ASSERT(ptr);
    auto& win = *static_cast<window*>(ptr);
    if (win._callbacks.viewport) {
      std::invoke(win._callbacks.viewport, win, extent2d{w, h});
    }
  }

  static void key_callback(GLFWwindow* handle, int key, int scancode, int action, int mod) {
    auto* ptr = glfwGetWindowUserPointer(handle);
    NTF_ASSERT(ptr);
    auto& win = *static_cast<window*>(ptr);
    if (win._callbacks.keypress) {
      std::invoke(win._callbacks.keypress, win, win_key_data{
        .key = static_cast<win_key>(key),
        .scancode = static_cast<win_keyscancode>(scancode),
        .action = static_cast<win_action>(action),
        .mod = static_cast<win_keymod>(mod)
      });
    }
  }

  static void cursor_pos_callback(GLFWwindow* handle, double xpos, double ypos) {
    auto* ptr = glfwGetWindowUserPointer(handle);
    NTF_ASSERT(ptr);
    auto& win = *static_cast<window*>(ptr);
    if (win._callbacks.cursorpos) {
      std::invoke(win._callbacks.cursorpos, win, dvec2{xpos, ypos});
    }
  }

  static void scroll_callback(GLFWwindow* handle, double xoff, double yoff) {
    auto* ptr = glfwGetWindowUserPointer(handle);
    NTF_ASSERT(ptr);
    auto& win = *static_cast<window*>(ptr);
    if (win._callbacks.scroll) {
      std::invoke(win._callbacks.scroll, win, dvec2{xoff, yoff});
    }
  }

  static void cursor_enter_callback(GLFWwindow* handle, int enters) {
    auto* ptr = glfwGetWindowUserPointer(handle);
    NTF_ASSERT(ptr);
    auto& win = *static_cast<window*>(ptr);
    if (win._callbacks.cursorenter) {
      std::invoke(win._callbacks.cursorenter, win, static_cast<bool>(enters));
    }
  }

  static void char_callback(GLFWwindow* handle, uint32 codepoint) {
    auto* ptr = glfwGetWindowUserPointer(handle);
    NTF_ASSERT(ptr);
    auto& win = *static_cast<window*>(ptr);
    if (win._callbacks.char_input) {
      std::invoke(win._callbacks.char_input, win, codepoint);
    }
  }

  static void button_callback(GLFWwindow* handle, int button, int action, int mods) {
    auto* ptr = glfwGetWindowUserPointer(handle);
    NTF_ASSERT(ptr);
    auto& win = *static_cast<window*>(ptr);
    if (win._callbacks.buttpress) {
      std::invoke(win._callbacks.buttpress, win, win_button_data{
        .button = static_cast<win_button>(button),
        .action = static_cast<win_action>(action),
        .mod = static_cast<win_keymod>(mods),
      });
    }
  }
#endif
};

static std::atomic<uint32> win_count = 0;

context_gl_params window::make_gl_params(window_t handle) noexcept {
  return {
    .gl_ctx = handle,
    .get_proc_address = +[](void* win, const char* name) -> void* {
      GLFWwindow* w = static_cast<GLFWwindow*>(win);
      NTF_UNUSED(w);
      glfwMakeContextCurrent(w);
      return reinterpret_cast<void*>(glfwGetProcAddress(name));
    },
    .swap_buffers = +[](void* win) -> void {
      GLFWwindow* w = static_cast<GLFWwindow*>(win);
      glfwSwapBuffers(w);
    },
    .make_current = +[](void* win)-> void {
      GLFWwindow* w = static_cast<GLFWwindow*>(win);
      glfwMakeContextCurrent(w);
    },
    .get_fb_size = +[](void* win, u32* width, u32* height) -> void {
      GLFWwindow* w = static_cast<GLFWwindow*>(win);
      int _w, _h;
      glfwGetFramebufferSize(w, &_w, &_h);
      *width = static_cast<u32>(_w);
      *height = static_cast<u32>(_h);
    },
  };
}

context_gl_params window::make_gl_params(const window& win) noexcept {
  return make_gl_params(win._handle);
}

win_expect<window> window::create(const win_params& params) {
#if defined(SHOGLE_ENABLE_GLFW) && SHOGLE_ENABLE_GLFW
  if (win_count.load() == 0) {
    if (!glfwInit()) {
      const char* err;
      glfwGetError(&err);
      RENDER_ERROR_LOG("Failed to initialize GLFW: {}", err);
      return ntf::unexpected{win_error{err}};
    }
    RENDER_DBG_LOG("GLFW initialized");
  }

  const char* title = params.title ? params.title : "window - ShOGLE";
  u32 swap_interval = 0;

  auto setup_gl_hints = [&](const win_gl_params& gl_params) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_params.ver_major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_params.ver_minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const bool transparent_fbo = gl_params.fb_use_alpha;
    u32 msaa = gl_params.fb_msaa_level;
    if (msaa > 0) {
      if (transparent_fbo) {
        RENDER_WARN_LOG("Framebuffer alpha set, ignoring MSAA");
      } else {
        msaa = msaa > 64 ? 64 : round_pow2(msaa);
        glfwWindowHint(GLFW_SAMPLES, msaa);
      }
    }

    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, transparent_fbo);

    i32 depth_bits = 0, stencil_bits = 0;
    switch (gl_params.fb_buffer) {
      case fbo_buffer::depth24u_stencil8u:
        stencil_bits = 8;
        [[fallthrough]];
      case fbo_buffer::depth24u:
        depth_bits = 24;
        break;
      case fbo_buffer::depth32f_stencil8u:
        stencil_bits = 8;
        [[fallthrough]];
      case fbo_buffer::depth32f:
        depth_bits = 32;
        break;
      case fbo_buffer::depth16u:
        depth_bits = 16;
        break;
      case fbo_buffer::none:
        break;
    }
    glfwWindowHint(GLFW_DEPTH_BITS, depth_bits);
    glfwWindowHint(GLFW_STENCIL_BITS, stencil_bits);
    swap_interval = gl_params.swap_interval;
  };

  auto setup_x11_hints = [&](const win_x11_params& x11_params) {
    if (x11_params.class_name) {
      glfwWindowHintString(GLFW_X11_CLASS_NAME, x11_params.class_name);
    } else {
      glfwWindowHintString(GLFW_X11_CLASS_NAME, title);
    }

    if (x11_params.instance_name) {
      glfwWindowHintString(GLFW_X11_INSTANCE_NAME, x11_params.instance_name);
    } else {
      glfwWindowHintString(GLFW_X11_INSTANCE_NAME, title);
    }
  };

  NTF_ASSERT(params.renderer_params);
  switch (params.renderer_api) {
    case context_api::opengl: {
      const win_gl_params& gl_params = *static_cast<const win_gl_params*>(params.renderer_params);
      setup_gl_hints(gl_params);
      break;
    }
    case context_api::vulkan:   [[fallthrough]];
    case context_api::software: [[fallthrough]];
    case context_api::none: {
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      break;
    }
  }

  // We only handle X11 for now
  if (params.platform_params) {
    const win_x11_params& x11_params = *static_cast<const win_x11_params*>(params.platform_params);
    setup_x11_hints(x11_params);
  }

  GLFWwindow* handle = glfwCreateWindow(params.width, params.height, title, nullptr, nullptr);
  if (!handle) {
    if (win_count.load() == 0) {
      glfwTerminate();
      RENDER_DBG_LOG("GLFW terminated");
    }

    const char* err;
    glfwGetError(&err);
    RENDER_ERROR_LOG("Failed to create GLFW window: {}", err);
    return ntf::unexpected{win_error{err}};
  }
  ++win_count;
  glfwMakeContextCurrent(handle);
  glfwSwapInterval(swap_interval);

  return win_expect<window>{ntf::in_place, win_cast(handle), context_api::opengl, params.attrib};
#endif
}

window::window(window_t handle, context_api ctx_api, win_attrib attrib) noexcept :
  _handle{handle}, _ctx_api{ctx_api}, _attrib{}
{ 
  GLFWwindow* win = win_cast(_handle);
  glfwSetFramebufferSizeCallback(win, callback_handler_t::fb_size_callback);
  glfwSetKeyCallback(win, callback_handler_t::key_callback);
  glfwSetCursorPosCallback(win, callback_handler_t::cursor_pos_callback);
  glfwSetCursorEnterCallback(win, callback_handler_t::cursor_enter_callback);
  glfwSetScrollCallback(win, callback_handler_t::scroll_callback);
  glfwSetCharCallback(win, callback_handler_t::char_callback);
  glfwSetMouseButtonCallback(win, callback_handler_t::button_callback);
  glfwSetWindowUserPointer(win, this);
  attribs(attrib); // Setup attributes here
}

window::~window() noexcept { _destroy(); }

void window::_destroy() {
#if defined(SHOGLE_ENABLE_GLFW) && SHOGLE_ENABLE_GLFW
  if (!_handle) {
    return;
  }
  glfwDestroyWindow(win_cast(_handle));
  RENDER_DBG_LOG("Window destroyed");
  if (--win_count == 0) {
    glfwTerminate();
    RENDER_DBG_LOG("GLFW terminated");
  }
#endif
}

window::window(window&& other) noexcept :
  _handle{std::move(other._handle)}, _ctx_api{std::move(other._ctx_api)},
  _attrib{std::move(other._attrib)},
  _callbacks{std::move(other._callbacks)}
{
  other._handle = nullptr;
  glfwSetWindowUserPointer(win_cast(_handle), this);
}

window& window::operator=(window&& other) noexcept {
  if (std::addressof(other) == this) {
    return *this;
  }

  _destroy();

  _handle = std::move(other._handle);
  _ctx_api = std::move(other._ctx_api);
  _attrib = std::move(other._attrib);
  _callbacks = std::move(other._callbacks);

  other._handle = nullptr;
  glfwSetWindowUserPointer(win_cast(_handle), this);

  return *this;
}

void window::close() {
#if defined(SHOGLE_ENABLE_GLFW) && SHOGLE_ENABLE_GLFW
  glfwSetWindowShouldClose(win_cast(_handle), 1);
#endif
}

void window::title(const std::string& title) {
#if defined(SHOGLE_ENABLE_GLFW) && SHOGLE_ENABLE_GLFW
  glfwSetWindowTitle(win_cast(_handle), title.c_str());
#endif
}

bool window::should_close() const {
#if defined(SHOGLE_ENABLE_GLFW) && SHOGLE_ENABLE_GLFW
  NTF_ASSERT(_handle);
  return glfwWindowShouldClose(win_cast(_handle));
#endif
}

win_action window::poll_key(win_key key) const {
#if defined(SHOGLE_ENABLE_GLFW) && SHOGLE_ENABLE_GLFW
  NTF_ASSERT(_handle);
  return static_cast<win_action>(glfwGetKey(win_cast(_handle), static_cast<int>(key)));
#endif
}

win_action window::poll_button(win_button button) const {
#if defined(SHOGLE_ENABLE_GLFW) && SHOGLE_ENABLE_GLFW
  NTF_ASSERT(_handle);
  return static_cast<win_action>(glfwGetMouseButton(win_cast(_handle), static_cast<int>(button)));
#endif
}

uvec2 window::win_size() const {
#if defined(SHOGLE_ENABLE_GLFW) && SHOGLE_ENABLE_GLFW
  NTF_ASSERT(_handle);
  int w, h;
  glfwGetWindowSize(win_cast(_handle), &w, &h);
  return uvec2{static_cast<uint32>(w), static_cast<uint32>(h)};
#endif
}

uvec2 window::fb_size() const {
#if defined(SHOGLE_ENABLE_GLFW) && SHOGLE_ENABLE_GLFW
  NTF_ASSERT(_handle);
  int w, h;
  glfwGetFramebufferSize(win_cast(_handle), &w, &h);
  return uvec2{static_cast<uint32>(w), static_cast<uint32>(h)};
#endif
}

void window::poll_events() {
#if defined(SHOGLE_ENABLE_GLFW) && SHOGLE_ENABLE_GLFW
  glfwPollEvents();
#endif
}

void window::attribs(win_attrib attrib) {
#if defined(SHOGLE_ENABLE_GLFW) && SHOGLE_ENABLE_GLFW
  glfwSetWindowAttrib(win_cast(_handle), GLFW_DECORATED, +(attrib & win_attrib::decorate));
  glfwSetWindowAttrib(win_cast(_handle), GLFW_RESIZABLE, +(attrib & win_attrib::resizable));
  glfwSetWindowAttrib(win_cast(_handle), GLFW_FLOATING, +(attrib & win_attrib::floating));
  glfwSetWindowAttrib(win_cast(_handle), GLFW_FOCUS_ON_SHOW, +(attrib & win_attrib::show_focus));
  _attrib = attrib;
#endif
}

} // namespace shogle
