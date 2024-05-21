#pragma once

#include <shogle/render/api/glfw.hpp>

namespace ntf::shogle {

class application {
public:
  application(size_t w, size_t h, std::string title);
  virtual ~application();

public:
  virtual void render() = 0;
  virtual void update(float) {}
  virtual void viewport_event() {}
  virtual void input_event() {}
  virtual void cursor_event() {}

public:
  void main_loop();
  void terminate();

private:
  glfw::window _window;
  double _last_frame {0.0};
};


} // namespace ntf::shogle
