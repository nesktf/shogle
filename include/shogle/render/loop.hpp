#pragma once

#include <shogle/render/context.hpp>
#include <shogle/render/window.hpp>

#include <chrono>

namespace shogle {

namespace meta {

template<typename F>
concept delta_time_func = std::invocable<F, f64>; // f(dt) -> void

template<typename F>
concept fixed_render_func = std::invocable<F, f64, f64>; // f(dt, alpha) -> void

template<typename F>
concept fixed_update_func = std::invocable<F, u32>; // f(ups) -> void

template<typename T>
concept has_render_member = requires(T t) {
  { t.on_render(f64{}) } -> std::convertible_to<void>;
};

template<typename T>
concept has_fixed_render_member = requires(T t) {
  { t.on_render(f64{}, f64{}) } -> std::convertible_to<void>;
};

template<typename T>
concept has_fixed_update_member = requires(T t) {
  { t.on_fixed_update(u32{}) } -> std::convertible_to<void>;
};

template<typename T>
concept fixed_loop_object = 
  (fixed_render_func<T> && fixed_update_func<T>) ||
  (has_fixed_render_member<T> && has_fixed_update_member<T>);

template<typename T>
concept nonfixed_loop_object = delta_time_func<T> || has_render_member<T>;

} // namespace meta

template<meta::nonfixed_loop_object LoopObj>
void render_loop(window& win, context_view ctx, LoopObj&& obj) {
  using namespace std::literals;

  using clock = std::chrono::steady_clock;
  using duration = clock::duration;
  using time_point = std::chrono::time_point<clock, duration>;

  time_point last_time = clock::now();
  while (!win.should_close()) {
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
    last_time = start_time;

    f64 dt {std::chrono::duration<f64>{elapsed_time}/1s};

    win.poll_events();

    ctx.start_frame();
    if constexpr (meta::has_render_member<LoopObj>) {
      obj.on_render(dt);
    } else {
      obj(dt);
    }
    ctx.end_frame();
  }
  ctx.device_wait();
}

template<meta::fixed_loop_object LoopObj>
void render_loop(window& win, context_view ctx, const u32& ups, LoopObj&& obj) {
  using namespace std::literals;

  auto fixed_elapsed_time =
    std::chrono::duration<f64>{std::chrono::microseconds{1000000/ups}};

  using clock = std::chrono::steady_clock;
  using duration = decltype(clock::duration{} + fixed_elapsed_time);
  using time_point = std::chrono::time_point<clock, duration>;

  time_point last_time = clock::now();
  duration lag = 0s;
  while (!win.should_close()) {
    fixed_elapsed_time = std::chrono::duration<f64>{std::chrono::microseconds{1000000/ups}};
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
    last_time = start_time;
    lag += elapsed_time;

    f64 dt {std::chrono::duration<f64>{elapsed_time}/1s};
    f64 alpha {std::chrono::duration<f64>{lag}/fixed_elapsed_time};

    win.poll_events();

    ctx.start_frame();

    while (lag >= fixed_elapsed_time) {
      if constexpr (meta::has_fixed_update_member<LoopObj>) {
        obj.on_fixed_update(ups);
      } else {
        obj(ups);
      }
      lag -= fixed_elapsed_time;
    }

    if constexpr (meta::has_fixed_render_member<LoopObj>) {
      obj.on_render(dt, alpha);
    } else {
      obj(dt, alpha);
    }

    ctx.end_frame();
  }
  ctx.device_wait();
}

template<meta::fixed_render_func RFunc, meta::fixed_update_func UFunc>
void render_loop(window& win, context_view ctx, const u32& ups,
                 RFunc&& render, UFunc&& fixed_update) {
  using namespace std::literals;

  auto fixed_elapsed_time =
    std::chrono::duration<f64>{std::chrono::microseconds{1000000/ups}};

  using clock = std::chrono::steady_clock;
  using duration = decltype(clock::duration{} + fixed_elapsed_time);
  using time_point = std::chrono::time_point<clock, duration>;

  time_point last_time = clock::now();
  duration lag = 0s;
  while (!win.should_close()) {
    fixed_elapsed_time = std::chrono::duration<f64>{std::chrono::microseconds{1000000/ups}};
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
    last_time = start_time;
    lag += elapsed_time;

    f64 dt {std::chrono::duration<f64>{elapsed_time}/1s};
    f64 alpha {std::chrono::duration<f64>{lag}/fixed_elapsed_time};

    win.poll_events();

    ctx.start_frame();

    while (lag >= fixed_elapsed_time) {
      fixed_update(ups);
      lag -= fixed_elapsed_time;
    }

    render(dt, alpha);

    ctx.end_frame();
  }
  ctx.device_wait();
}

} // namespace shogle
