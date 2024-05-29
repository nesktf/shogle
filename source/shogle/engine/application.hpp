#pragma once

#include <shogle/render/api/glfw.hpp>

namespace ntf::shogle {

class application {
public:
  application(size_t w, size_t h, std::string title);
  virtual ~application();

public:
  virtual void draw_event() = 0;
  virtual void update_event(float) {}
  virtual void viewport_event(size_t, size_t) {}
  virtual void input_event(int, int) {}
  virtual void cursor_event(double, double) {}
  virtual void scroll_event(double, double) {}

public:
  void main_loop();
  void terminate();

public:
  vec2sz win_size() const { return glfw::window_size(_window); }
  bool is_key_pressed(int glfw_key) const { return glfw::is_key_pressed(_window, glfw_key); }

protected:
  glfw::window _window;
  double _last_frame {0.0};
};


} // namespace ntf::shogle
