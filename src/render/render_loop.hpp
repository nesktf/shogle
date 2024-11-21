#pragma once

#include "../core.hpp"

#include <chrono>

namespace ntf {

template<typename F>
concept render_func = std::invocable<F, double, double>; // f(dt, alpha) -> void

template<typename F>
concept update_func = std::invocable<F>; // f() -> void

template<typename W>
concept window_object = true; // TODO: define this

template<window_object Window, render_func RFunc, update_func UFunc>
void shogle_main_loop(Window& window, uint ups, RFunc&& render, UFunc&& fixed_update) {
  using namespace std::literals;

  const auto fixed_elapsed_time =
    std::chrono::duration<double>{std::chrono::microseconds{1000000/ups}};

  using clock = std::chrono::steady_clock;
  using duration = decltype(clock::duration{} + fixed_elapsed_time);
  using time_point = std::chrono::time_point<clock, duration>;

  SHOGLE_INTERNAL_LOG_FMT(debug, "[SHOGLE][ntf::shogle_main_loop] Main loop started (ups: {})",
                          ups);

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

  SHOGLE_INTERNAL_LOG(debug, "[SHOGLE][ntf::shogle_main_loop] Main loop exit");
}

} // namespace ntf
