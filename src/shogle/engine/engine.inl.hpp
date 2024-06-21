#define ENGINE_INL_HPP
#include <shogle/engine/engine.hpp>
#undef ENGINE_INL_HPP

namespace ntf::shogle {

template<drawfun T>
engine& engine::set_draw_event(T&& fun) {
  _draw_event = std::forward<T>(fun);
  return *this;
}

template<updatefun T>
engine& engine::set_update_event(T&& fun) {
  _update_event = std::forward<T>(fun);
  return *this;
}

template<viewportfun T>
engine& engine::set_viewport_event(T&& fun) {
  _window.viewport_event.set_callback(
    [fun=std::forward<T>(fun)](auto&, size_t w, size_t h) {
      fun(w, h);
    }
  );
  return *this;
}

template<keyfun T>
engine& engine::set_key_event(T&& fun) {
  _window.key_event.set_callback(
    [fun=std::forward<T>(fun)](auto&, keycode key, scancode scan, keystate state, keymod mod) {
      fun(key, scan, state, mod);
    }
  );
  return *this;
}

template<cursorfun T>
engine& engine::set_cursor_event(T&& fun) {
  _window.cursor_event.set_callback(
    [fun=std::forward<T>(fun)](auto&, double xpos, double ypos) {
      fun(xpos, ypos);
    }
  );
  return *this;
}

template<scrollfun T>
engine& engine::set_scroll_event(T&& fun) {
  _window.scroll_event.set_callback(
    [fun=std::forward<T>(fun)](auto&, double xoff, double yoff) {
      fun(xoff, yoff);
    }
  );
  return *this;
}

} // namespace ntf::shogle
