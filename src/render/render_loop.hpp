#pragma once

#include "../core.hpp"

#include <chrono>

namespace ntf {

template<typename T>
concept window_object = requires(T win) {
  { win.should_close() } -> std::convertible_to<bool>;
  { win.poll_events() } -> std::convertible_to<void>;
  { win.swap_buffers() } -> std::convertible_to<void>;
};


template<typename F>
concept delta_time_func = std::invocable<F, double>; // f(dt) -> void

template<typename F>
concept fixed_render_func = std::invocable<F, double, double>; // f(dt, alpha) -> void

template<typename F>
concept fixed_update_func = std::invocable<F>; // f() -> void


template<typename T>
concept has_render_member = requires(T t) {
  { t.on_render(double{}) } -> std::convertible_to<void>;
};

template<typename T>
concept has_fixed_render_member = requires(T t) {
  { t.on_render(double{}, double{}) } -> std::convertible_to<void>;
};

template<typename T>
concept has_fixed_update_member = requires(T t) {
  { t.on_fixed_update() } -> std::convertible_to<void>;
};


template<typename T>
concept fixed_loop_object = 
  (fixed_render_func<T> && fixed_update_func<T>) ||
  (has_fixed_render_member<T> && has_fixed_update_member<T>);

template<typename T>
concept nonfixed_loop_object = delta_time_func<T> || has_render_member<T>;


template<window_object Window, nonfixed_loop_object LoopObj>
void shogle_main_loop(Window& window, LoopObj&& obj) {
  using namespace std::literals;

  using clock = std::chrono::steady_clock;
  using duration = clock::duration;
  using time_point = std::chrono::time_point<clock, duration>;

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Main loop started");

  time_point last_time = clock::now();
  while (!window.should_close()) {
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
    last_time = start_time;

    double dt {std::chrono::duration<double>{elapsed_time}/1s};

    window.poll_events();
    if constexpr (has_render_member<LoopObj>) {
      obj.on_render(dt);
    } else {
      obj(dt);
    }
    window.swap_buffers();
  }

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Main loop exit");
}

template<window_object Window, fixed_loop_object LoopObj>
void shogle_main_loop(Window& window, uint ups, LoopObj&& obj) {
  using namespace std::literals;

  const auto fixed_elapsed_time =
    std::chrono::duration<double>{std::chrono::microseconds{1000000/ups}};

  using clock = std::chrono::steady_clock;
  using duration = decltype(clock::duration{} + fixed_elapsed_time);
  using time_point = std::chrono::time_point<clock, duration>;

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Fixed main loop started at {} ups", ups);

  time_point last_time = clock::now();
  duration lag = 0s;
  while (!window.should_close()) {
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
    last_time = start_time;
    lag += elapsed_time;

    double dt {std::chrono::duration<double>{elapsed_time}/1s};
    double alpha {std::chrono::duration<double>{lag}/fixed_elapsed_time};

    window.poll_events();

    while (lag >= fixed_elapsed_time) {
      if constexpr (has_fixed_update_member<LoopObj>) {
        obj.on_fixed_update();
      } else {
        obj();
      }
      lag -= fixed_elapsed_time;
    }

    if constexpr (has_fixed_render_member<LoopObj>) {
      obj.on_render(dt, alpha);
    } else {
      obj(dt, alpha);
    }

    window.swap_buffers();
  }

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Main loop exit");
}

template<window_object Window, fixed_render_func RFunc, fixed_update_func UFunc>
void shogle_main_loop(Window& window, uint ups, RFunc&& render, UFunc&& fixed_update) {
  using namespace std::literals;

  const auto fixed_elapsed_time =
    std::chrono::duration<double>{std::chrono::microseconds{1000000/ups}};

  using clock = std::chrono::steady_clock;
  using duration = decltype(clock::duration{} + fixed_elapsed_time);
  using time_point = std::chrono::time_point<clock, duration>;

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Fixed main loop started at {} ups", ups);

  time_point last_time = clock::now();
  duration lag = 0s;
  while (!window.should_close()) {
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
    last_time = start_time;
    lag += elapsed_time;

    double dt {std::chrono::duration<double>{elapsed_time}/1s};
    double alpha {std::chrono::duration<double>{lag}/fixed_elapsed_time};

    window.poll_events();

    while (lag >= fixed_elapsed_time) {
      fixed_update();
      lag -= fixed_elapsed_time;
    }

    render(dt, alpha);

    window.swap_buffers();
  }

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Main loop exit");
}

} // namespace ntf
