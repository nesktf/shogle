#define ENGINE_INL_HPP
#include <shogle/engine/engine.hpp>
#undef ENGINE_INL_HPP

#include <shogle/core/log.hpp>

#include <thread>
#include <chrono>

namespace ntf::shogle {

template<renderfunc RFunc, updatefunc FUFunc, updatefunc UFunc>
void engine::start(uint ups, RFunc&& render, FUFunc&& fixed_update, UFunc&& update) {
  using namespace std::literals;
  using clock = std::chrono::steady_clock;

  auto fixed_elapsed_time = std::chrono::duration<double>{std::chrono::microseconds{1000000/ups}};
  using duration = decltype(clock::duration{} + fixed_elapsed_time);
  using time_point = std::chrono::time_point<clock, duration>;

  log::debug("[shogle::engine] Starting main loop at {} ups", ups);

  time_point last_time = clock::now();
  duration lag = 0s;
  while (_window.is_open()) {
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
    last_time = start_time;
    lag += elapsed_time;

    _imgui.new_frame();
    _window.poll_events();

    while (lag >= fixed_elapsed_time) {
      double dt = std::chrono::duration<double>{fixed_elapsed_time}/1s;
      fixed_update(_window, dt);
      lag -= fixed_elapsed_time;
    }

    double dt {std::chrono::duration<double>{elapsed_time}/1s};
    double alpha {std::chrono::duration<double>{lag}/fixed_elapsed_time};
    update(_window, dt);
    render(_window, dt, alpha);

    _imgui.render();
    _window.swap_buffers();
  }

  log::debug("[shogle::engine] Main loop exited");
}

template<renderfunc RFunc, updatefunc FUFunc>
void engine::start(uint ups, RFunc&& render, FUFunc&& fixed_update) {
  engine::start(ups, std::forward<RFunc>(render), std::forward<FUFunc>(fixed_update), [](auto&,auto){});
}

template<viewportfunc F>
engine& engine::set_viewport_event(F&& fun) {
  _window.viewport_event.set_callback(
    [fun=std::forward<F>(fun)](auto&, size_t w, size_t h) {
      fun(w, h);
    }
  );
  return *this;
}

template<keyfunc F>
engine& engine::set_key_event(F&& fun) {
  _window.key_event.set_callback(
    [fun=std::forward<F>(fun)](auto&, keycode key, scancode scan, keystate state, keymod mod) {
      fun(key, scan, state, mod);
    }
  );
  return *this;
}

template<cursorfunc F>
engine& engine::set_cursor_event(F&& fun) {
  _window.cursor_event.set_callback(
    [fun=std::forward<F>(fun)](auto&, double xpos, double ypos) {
      fun(xpos, ypos);
    }
  );
  return *this;
}

template<scrollfunc F>
engine& engine::set_scroll_event(F&& fun) {
  _window.scroll_event.set_callback(
    [fun=std::forward<F>(fun)](auto&, double xoff, double yoff) {
      fun(xoff, yoff);
    }
  );
  return *this;
}

} // namespace ntf::shogle
