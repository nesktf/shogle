#pragma once

#include <shogle/shogle.hpp>

// Include all the main things
#include <shogle/render/gl.hpp>
#include <shogle/render/glfw.hpp>
#include <shogle/render/imgui.hpp>

namespace ntf {

template<typename F>
concept fixrenderfunc = std::invocable<F, double, double>;

template<typename F>
concept renderfunc = std::invocable<F>;

template<typename F>
concept updatefunc = std::invocable<F, double>;

template<typename F>
concept fupdatefunc = std::invocable<F>;

template<typename Window, typename ImguiImpl, fixrenderfunc RFunc, fupdatefunc FUFunc>
void shogle_main_loop(Window& window, const imgui::imgui_lib<ImguiImpl>& imgui,
                      uint ups, RFunc&& render, FUFunc&& fixed_update) {
  using namespace std::literals;

  const auto fixed_elapsed_time = std::chrono::duration<double>{std::chrono::microseconds{1000000/ups}};

  using clock = std::chrono::steady_clock;
  using duration = decltype(clock::duration{} + fixed_elapsed_time);
  using time_point = std::chrono::time_point<clock, duration>;

  SHOGLE_INTERNAL_LOG_FMT(debug, "[SHOGLE][ntf::shogle_main_loop] Main loop started (ups: {})", ups);

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

    imgui.start_frame();

    render(dt, alpha);

    imgui.end_frame();

    window.swap_buffers();
  }

  SHOGLE_INTERNAL_LOG(debug, "[SHOGLE][ntf::shogle_main_loop] Main loop exit");
}

template<typename Window, fixrenderfunc RFunc, fupdatefunc FUFunc>
void shogle_main_loop(Window& window, uint ups, RFunc&& render, FUFunc&& fixed_update) {
  using namespace std::literals;

  const auto fixed_elapsed_time = std::chrono::duration<double>{std::chrono::microseconds{1000000/ups}};

  using clock = std::chrono::steady_clock;
  using duration = decltype(clock::duration{} + fixed_elapsed_time);
  using time_point = std::chrono::time_point<clock, duration>;

  SHOGLE_INTERNAL_LOG_FMT(debug, "[SHOGLE][ntf::shogle_main_loop] Main loop started (ups: {})", ups);

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
