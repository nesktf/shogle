#pragma once

#include <shogle/render/gl/common.hpp>

#ifndef SHOGLE_DISABLE_GLFW
#include <GLFW/glfw3.h>
#endif

namespace shogle {

#ifndef SHOGLE_DISABLE_GLFW
using glfw_enum = int;

struct glfw_key_data {
  glfw_enum key;
  glfw_enum scancode;
  glfw_enum action;
  glfw_enum modifiers;
};

struct glfw_button_data {
  glfw_enum button;
  glfw_enum action;
  glfw_enum modifiers;
};

namespace impl {

template<typename T>
struct glfw_context {
public:
  template<typename Signature>
  using callback_type = std::function<Signature>;

  using viewport_fun = callback_type<void(T&, extent2d)>;
  using key_input_fun = callback_type<void(T&, glfw_key_data)>;
  using cursor_pos_fun = callback_type<void(T&, f64, f64)>;
  using cursor_enter_fun = callback_type<void(T&, bool)>;
  using scroll_fun = callback_type<void(T&, f64, f64)>;
  using mouse_input_fun = callback_type<void(T&, glfw_button_data)>;
  using char_input_fun = callback_type<void(T&, u32)>;

public:
  glfw_context(GLFWwindow* win);

  NTF_DECLARE_NO_MOVE_NO_COPY(glfw_context);

public:
  GLFWwindow* win;
  viewport_fun on_viewport;
  key_input_fun on_key_input;
  cursor_pos_fun on_cursor_pos;
  cursor_enter_fun on_cursor_enter;
  scroll_fun on_scroll;
  mouse_input_fun on_mouse_input;
  char_input_fun on_char_input;
};

} // namespace impl

class glfw_gl_context : public gl_surface_provider {
private:
  using ctx_t = ::shogle::impl::glfw_context<glfw_gl_context>;

  struct create_t {};

public:
  enum depth_bits {
    DEPTH_BITS_NONE = 0,
    DEPTH_BITS_24U = 24,
    DEPTH_BITS_32F = 32,
  };

  enum stencil_bits {
    STENCIL_BITS_NONE = 0,
    STENCIL_BITS_8U = 8,
  };

  enum alpha_mode {
    ALPHA_DISABLE = 0,
    ALPHA_ENABLE = 1,
  };

  struct x11_hints {
    const char* class_name;
    const char* instance_name;
  };

  struct window_hints {
    ::shogle::gl_version gl_ver;
    depth_bits depth_buffer;
    stencil_bits stencil_buffer;
    alpha_mode window_alpha;
    u32 msaa_samples;
    ntf::optional<x11_hints> x11;
  };

public:
  glfw_gl_context(create_t, unique_ptr<ctx_t>&& ctx) noexcept;

  glfw_gl_context(const window_hints& hints, extent2d window_size, const char* window_name,
                  GLFWmonitor* monitor = nullptr, GLFWwindow* share = nullptr);

public:
  static sv_expect<glfw_gl_context> create(const window_hints& hints, extent2d window_size,
                                           const char* name, GLFWmonitor* monitor = nullptr,
                                           GLFWwindow* share = nullptr);

public:
  shogle::PFN_glGetProcAddress proc_loader() noexcept override {
    return reinterpret_cast<shogle::PFN_glGetProcAddress>(glfwGetProcAddress);
  }

  shogle::extent2d surface_extent() const noexcept override {
    if (NTF_UNLIKELY(!_ctx)) {
      return {.width = 0, .height = 0};
    }
    int w, h;
    glfwGetFramebufferSize(_ctx->win, &w, &h);
    return {.width = (u32)w, .height = (u32)h};
  }

  void swap_buffers() noexcept override {
    if (NTF_UNLIKELY(!_ctx)) {
      return;
    }
    glfwSwapBuffers(_ctx->win);
  }

public:
  void destroy() {
    NTF_ASSERT(_ctx);
    _ctx.reset();
  }

  GLFWwindow* get() const {
    NTF_ASSERT(_ctx);
    return _ctx->win;
  }

public:
  operator GLFWwindow*() const { return get(); }

public:
  template<typename F>
  glfw_gl_context& set_viewport_callback(F&& func) {
    NTF_ASSERT(_ctx);
    _ctx->on_viewport = std::forward<F>(func);
    return *this;
  }

  void remove_viewport_callback() {
    NTF_ASSERT(this->_ctx);
    this->_ctx->on_viewport = {};
  }

  template<typename F>
  glfw_gl_context& set_key_input_callback(F&& func) {
    NTF_ASSERT(_ctx);
    _ctx->on_key_input = std::forward<F>(func);
    return *this;
  }

  void remove_key_input_callback() {
    NTF_ASSERT(_ctx);
    _ctx->on_viewport = {};
  }

  template<typename F>
  glfw_gl_context& set_cursor_pos_callback(F&& func) {
    NTF_ASSERT(_ctx);
    _ctx->on_cursor_pos = std::forward<F>(func);
    return *this;
  }

  void remove_cursor_pos_callback() {
    NTF_ASSERT(_ctx);
    _ctx->on_cursor_pos = {};
  }

  template<typename F>
  glfw_gl_context& set_cursor_enter_callback(F&& func) {
    NTF_ASSERT(_ctx);
    _ctx->on_cursor_enter = std::forward<F>(func);
    return *this;
  }

  void remove_cursor_enter_callback() {
    NTF_ASSERT(this->_ctx);
    _ctx->on_cursor_enter = {};
  }

  template<typename F>
  glfw_gl_context& set_scroll_callback(F&& func) {
    NTF_ASSERT(_ctx);
    _ctx->on_scroll = std::forward<F>(func);
    return *this;
  }

  void remove_scroll_callback() {
    NTF_ASSERT(this->_ctx);
    _ctx->on_scroll = {};
  }

  template<typename F>
  glfw_gl_context& set_mouse_input_callback(F&& func) {
    NTF_ASSERT(_ctx);
    _ctx->on_mouse_input = std::forward<F>(func);
    return *this;
  }

  void remove_mouse_input_callback() {
    NTF_ASSERT(_ctx);
    _ctx->on_mouse_input = {};
  }

  template<typename F>
  glfw_gl_context& set_char_input_callback(F&& func) {
    NTF_ASSERT(_ctx);
    _ctx->on_mouse_input = std::forward<F>(func);
    return *this;
  }

  void remove_char_input_callback() {
    NTF_ASSERT(_ctx);
    _ctx->on_char_input = {};
  }

private:
  unique_ptr<ctx_t> _ctx;
};
#endif

} // namespace shogle
