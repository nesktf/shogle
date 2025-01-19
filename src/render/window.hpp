#pragma once

#include <glad/glad.h>

#include "./render.hpp"
#include "./keys.hpp"
#include "../stl/function.hpp"
#include "../stl/optional.hpp"

namespace ntf {

struct r_window_params {
  uint32 width = 1280;
  uint32 height = 720;
  std::string_view title = "test";

  std::string_view x11_class_name;
  std::string_view x11_instance_name;
};

class r_window {
private:
  struct ctx_params_t {
    r_api api;
    uint32 gl_maj;
    uint32 gl_min;
    optional<uint32> swap_interval;
  };

public:
  using viewport_fun = ntf::inplace_function<void(uint32,uint32)>;
  using key_fun = ntf::inplace_function<void(keycode,scancode,keystate,keymod)>;
  using cursor_fun = ntf::inplace_function<void(float64,float64)>;
  using scroll_fun = ntf::inplace_function<void(float64,float64)>;

public:
  r_window(const r_window_params& params = {}) :
    _init_params(params) {}

public:
  void viewport_event(viewport_fun callback);
  void key_event(key_fun callback);
  void cursor_event(cursor_fun callback);
  void scroll_event(scroll_fun callback);

  void title(std::string_view title);

  void close();

  void poll_events();

public:
  bool should_close() const;
  bool poll_key(keycode key, keystate state) const;
  uvec2 win_size() const;
  uvec2 fb_size() const;

  [[nodiscard]] bool valid() const { return _handle != nullptr; }
  explicit operator bool() const { return valid(); }

#if SHOGLE_USE_GLFW
  GLADloadproc proc_loader() {
    return reinterpret_cast<GLADloadproc>(glfwGetProcAddress);
  }
  bool create_surface(VkInstance instance, VkSurfaceKHR* surface,
                      const VkAllocationCallbacks* alloc) {
    return glfwCreateWindowSurface(instance, _handle, alloc, surface);
  }
#endif

private:
  void swap_buffers();
  bool init_context(ctx_params_t ctx_params);
  void reset();

private:
#if SHOGLE_USE_GLFW
  static void fb_callback(GLFWwindow* handle, int w, int h);
  static void key_callback(GLFWwindow* handle, int code, int scan, int state, int mod);
  static void cursor_callback(GLFWwindow* handle, double xpos, double ypos);
  static void scroll_callback(GLFWwindow* handle, double xoff, double yoff);
#endif

private:
#if SHOGLE_USE_GLFW
  GLFWwindow* _handle{nullptr};
#endif
  r_window_params _init_params;
  r_api _ctx_api;
  viewport_fun _viewport_event;
  key_fun _key_event;
  cursor_fun _cursor_event;
  scroll_fun _scroll_event;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(r_window);

private:
  friend class r_context;
};

} // namespace ntf
