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
  virtual void viewport_event() {}
  virtual void input_event() {}
  virtual void cursor_event() {}

public:
  void main_loop();
  void terminate();

public:
  vec2sz win_size() const { return glfw::window_size(_window); }

private:
  glfw::window _window;
  double _last_frame {0.0};
};


} // namespace ntf::shogle
