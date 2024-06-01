#pragma once

#include <shogle/render/glfw/window.hpp>
#include <shogle/render/imgui/imgui.hpp>

#include <shogle/engine/event_traits.hpp>

namespace ntf::shogle {

class engine {
public:
  engine(size_t w, size_t h, std::string title);

public:
  void start();

public:
  template<drawfun T>
  engine& set_draw_event(T&& fun) {
    _draw_event = std::forward<T>(fun);
    return *this;
  }

  template<updatefun T>
  engine& set_update_event(T&& fun) {
    _update_event = std::forward<T>(fun);
    return *this;
  }

  template<viewportfun T>
  engine& set_viewport_event(T&& fun) {
    _window.viewport_event.set_callback(
      [fun=std::forward<T>(fun)](auto&, size_t w, size_t h) {
        fun(w, h);
      }
    );
    return *this;
  }

  template<keyfun T>
  engine& set_key_event(T&& fun) {
    _window.key_event.set_callback(
      [fun=std::forward<T>(fun)](auto&, glfw::keycode key, glfw::scancode scan, glfw::keystate state, glfw::keymod mod) {
        fun(key, scan, state, mod);
      }
    );
    return *this;
  }

  template<cursorfun T>
  engine& set_cursor_event(T&& fun) {
    _window.cursor_event.set_callback(
      [fun=std::forward<T>(fun)](auto&, double xpos, double ypos) {
        fun(xpos, ypos);
      }
    );
    return *this;
  }

  template<scrollfun T>
  engine& set_scroll_event(T&& fun) {
    _window.scroll_event.set_callback(
      [fun=std::forward<T>(fun)](auto&, double xoff, double yoff) {
        fun(xoff, yoff);
      }
    );
    return *this;
  }

public:
  glfw::window& window() { return _window; }

public:
  engine(engine&&) = delete;
  engine(const engine&) = delete;
  engine& operator=(engine&&) = delete;
  engine& operator=(const engine&) = delete;

private:
  glfw::window _window;
  imgui::renderer _imgui;

  std::function<void()> _draw_event;
  std::function<void(float)> _update_event;
};


} // namespace ntf::shogle
