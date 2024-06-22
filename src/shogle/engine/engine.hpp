#pragma once

#include <shogle/render/render.hpp>
#include <shogle/render/glfw/window.hpp>
#include <shogle/render/imgui/imgui.hpp>

#include <shogle/engine/event_traits.hpp>

namespace ntf::shogle {

class engine {
public:
  engine(size_t w, size_t h, std::string title) :
    _window(w, h, std::move(title)), _imgui(_window) {}

public:
  // RFunc  -> void(shogle::window&,double,double)
  // FUFunc -> void(shogle::window&,double)
  // UFunc  -> void(shogle::window&,double)
  template<renderfunc RFunc, updatefunc FUFunc, updatefunc UFunc>
  void start(uint ups, RFunc&& render, FUFunc&& fixed_update, UFunc&& update);

  template<renderfunc RFunc, updatefunc FUFunc>
  void start(uint ups, RFunc&& render, FUFunc&& fixed_update);

public:
  // void(size_t, size_t)
  template<viewportfunc F>
  engine& set_viewport_event(F&& fun);

  // void(keycode, scancode, keystate, keymod)
  template<keyfunc F>
  engine& set_key_event(F&& fun);

  // void(double, double)
  template<cursorfunc F>
  engine& set_cursor_event(F&& fun);

  // void(double, double)
  template<scrollfunc F>
  engine& set_scroll_event(F&& fun);

public:
  window& win() { return _window; }

public:
  // Should live in main(), can't move or copy :p
  engine(engine&&) = delete;
  engine(const engine&) = delete;
  engine& operator=(engine&&) = delete;
  engine& operator=(const engine&) = delete;

private:
  window _window;
  imgui_handle _imgui;
};

} // namespace ntf::shogle

#ifndef ENGINE_INL_HPP
#include <shogle/engine/engine.inl.hpp>
#endif
