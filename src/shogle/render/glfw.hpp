#pragma once

#include <shogle/math/alg.hpp>

#include <shogle/render/glfw_keys.hpp>

#include <shogle/render/imgui.hpp>

namespace ntf {

class glfw {
public:
  class glfw_lib;

  template<typename Renderer>
  class window;

public:
  using keycode = glfw_keys::keycode;
  using scancode = glfw_keys::scancode;
  using keystate = glfw_keys::keystate;
  using keymod = glfw_keys::keymod;

  using viewport_event = std::function<void(size_t,size_t)>;
  using key_event = std::function<void(keycode,scancode,keystate,keymod)>;
  using cursor_event = std::function<void(double, double)>;
  using scroll_event = std::function<void(double, double)>;
 
public:
  [[nodiscard]] static glfw_lib init();

  template<typename Renderer>
  static void set_current_context(const window<Renderer>& window);

  static void set_swap_interval(uint interval);

  static void poll_events();
};


class glfw::glfw_lib {
private:
  glfw_lib() = default;

private:
  friend class glfw;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(glfw_lib);
};


template<typename Renderer>
class glfw::window {
public:
  using renderer_type = Renderer;
  using imgui_impl = imgui::glfw_gl3_impl; // TODO: Change this?

public:
  window(size_t w, size_t h, std::string_view title) :
    window(ivec2{static_cast<int>(w), static_cast<int>(h)}, title) {}

  window(ivec2 sz, std::string_view title);

public:
  void set_viewport_event(viewport_event event);
  void set_key_event(key_event event);
  void set_cursor_event(cursor_event event);
  void set_scroll_event(scroll_event event);

  void set_title(std::string_view title);

  void close();

  void poll_events();
  void swap_buffers();

  void imgui_init(imgui_impl impl);

public:
  bool should_close() const;
  bool poll_key(keycode key, keystate state) const;
  ivec2 size() const;
  bool valid() const { return _handle != nullptr; }

  operator bool() const { return valid(); }

private:
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

private:
  friend class glfw;

public:
  NTF_DECLARE_MOVE_ONLY(window);
};

} // namespace ntf

#ifndef SHOGLE_RENDER_GLFW_INL
#include <shogle/render/glfw.inl>
#endif
