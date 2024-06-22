#pragma once

#include <shogle/core/types.hpp>

#include <shogle/render/glfw/keys.hpp>

#include <string>
#include <functional>

namespace ntf::shogle {

class imgui_handle;

class window {
public:
  template<typename... Args>
  class event {
  public:
    using callback_t = std::function<void(Args...)>;
  public:
    template<typename callback>
    void set_callback(callback&& cb) {
      _event = std::forward<callback>(cb);
    }
    void operator()(Args... args) {
      if (_event) { _event(args...); }
    }
  private:
    std::function<void(Args...)> _event;
  };

public:
  window(size_t w, size_t h, std::string title);

public:
  void close();

  window& set_title(std::string title);

public:
  inline window& poll_events() {
    glfwPollEvents();
    return *this;
  }

  inline window& swap_buffers() {
    glfwSwapBuffers(_handle);
    return *this;
  }

  inline double elapsed_time() const {
    return glfwGetTime();
  }

  inline bool get_key(keycode key) {
    return glfwGetKey(_handle, key) == GLFW_PRESS;
  }

  inline window& use_vsync(bool flag) {
    glfwSwapInterval(flag);
    return *this;
  }

public:
  vec2sz size() const;
  bool should_close() const { return glfwWindowShouldClose(_handle); }
  bool is_open() const { return !should_close(); }

public:
  ~window();
  window(window&&) = default;
  window(const window&) = delete;
  window& operator=(window&&) = default;
  window& operator=(const window&) = delete;

public:
  static void _framebuffer_size_cb(GLFWwindow* win, int w, int h);
  static void _key_event_cb(GLFWwindow* win, int code, int scan, int state, int mod);
  static void _cursor_event_cb(GLFWwindow* win, double xpos, double ypos);
  static void _scroll_event_cb(GLFWwindow* win, double xoff, double yoff);

public:
  event<window&,size_t,size_t> viewport_event;
  event<window&,keycode,scancode,keystate,keymod> key_event;
  event<window&,double,double> cursor_event;
  event<window&,double,double> scroll_event;

private:
  GLFWwindow* _handle;
  std::string _title;

  friend class imgui_handle;
};

} // namespace ntf::shogle
