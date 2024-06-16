#include <shogle/render/gl/gl.hpp>
#include <shogle/engine/engine.hpp>

#include <shogle/core/error.hpp>

#include <chrono>

namespace ntf::shogle {

engine::engine(size_t w, size_t h, std::string title) :
  _window(w, h, std::move(title)),
  _imgui(_window) { 
  log::debug("[shogle::engine] Engine created (w: {}, h: {})", w, h);
}

void engine::start() {
  if (!_draw_event || !_update_event) {
    throw ntf::error {"[shogle::engine::start] Draw and update events not properly set"};
  }
  log::debug("[shogle::engine] Main loop started");

  double _last_frame = 0.0;
  while (_window.is_open()) {
    _window.poll_events();
    _imgui.new_frame();

    double _curr_frame = _window.elapsed_time();
    double dt = _curr_frame - _last_frame;
    _last_frame = _curr_frame;

    _draw_event();
    _update_event(dt);

    _imgui.render();
    _window.swap_buffers();
  }

  log::debug("[shogle::engine] Main loop exited");
}

} // namespace ntf::shogle
