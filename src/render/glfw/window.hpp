#pragma once

#include "../render.hpp"
#include "./glfw.hpp"

#ifdef SHOGLE_ENABLE_IMGUI
#include "../imgui/imgui.hpp"
#include <imgui_impl_glfw.h>
#endif

#include <functional>

namespace ntf {

#ifdef SHOGLE_ENABLE_IMGUI
template<typename RenderImpl>
struct glfw_imgui_impl {
  static constexpr bool valid_impl = true;
  static constexpr renderer_backend backend_enum = RenderImpl::backend_enum;

  template<typename... Args>
  static bool init(GLFWwindow* win, bool callbacks, Args&&... args) {
    if constexpr (backend_enum == renderer_backend::opengl) {
      ImGui_ImplGlfw_InitForOpenGL(win, callbacks);
    } else if constexpr (backend_enum == renderer_backend::vulkan) {
      ImGui_ImplGlfw_InitForVulkan(win, callbacks);
    } else {
      ImGui_ImplGlfw_InitForOther(win, callbacks);
    }
    return RenderImpl::init(std::forward<Args>(args)...);
  }

  static void destroy() {
    RenderImpl::destroy();
    ImGui_ImplGlfw_Shutdown();
  }

  static void start_frame() {
    RenderImpl::start_frame();
    ImGui_ImplGlfw_NewFrame();
  }

  static void end_frame() {
    RenderImpl::end_frame();
  }
};

template<>
struct glfw_imgui_impl<void> {
  static constexpr bool valid_impl = false;
};
#endif

template<typename RenderContext>
class glfw_window {
public:
  using context_type = RenderContext;

#ifdef SHOGLE_ENABLE_IMGUI
  using imgui_impl = glfw_imgui_impl<typename RenderContext::imgui_impl>;
  static constexpr bool imgui_enabled = imgui_impl::valid_impl;
#endif

  using viewport_event = std::function<void(size_t,size_t)>;
  using key_event = std::function<void(glfw_keycode,glfw_scancode,glfw_keystate,glfw_keymod)>;
  using cursor_event = std::function<void(double, double)>;
  using scroll_event = std::function<void(double, double)>;

public:
  glfw_window(std::size_t w, std::size_t h, std::string_view title);

public:
  void set_viewport_event(viewport_event event);
  void set_key_event(key_event event);
  void set_cursor_event(cursor_event event);
  void set_scroll_event(scroll_event event);

  void set_title(std::string_view title);

  void close();

  void poll_events();
  void swap_buffers();

public:
  bool should_close() const;
  bool poll_key(glfw_keycode key, glfw_keystate state) const;
  ivec2 size() const;
  bool valid() const { return _handle != nullptr; }

  explicit operator bool() const { return valid(); }

public:
  static void fb_callback(GLFWwindow* win, int w, int h);
  static void key_callback(GLFWwindow* win, int code, int scan, int state, int mod);
  static void cursor_callback(GLFWwindow* win, double xpos, double ypos);
  static void scroll_callback(GLFWwindow* win, double xoff, double yoff);

private:
  GLFWwindow* _handle;
  viewport_event _viewport_event;
  key_event _key_event;
  cursor_event _cursor_event;
  scroll_event _scroll_event;
  context_type _render_ctx;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(glfw_window);
};

} // namespace ntf

#ifndef SHOGLE_RENDER_GLFW_WINDOW_INL
#include "./window.inl"
#endif
