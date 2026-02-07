#pragma once

#include <shogle/render/gl/context.hpp>
#include <shogle/render/window.hpp>

#include <chrono>

namespace shogle {

namespace meta {

template<typename F>
concept delta_render_func = std::invocable<F, f64>; // f(dt) -> void

template<typename T>
concept delta_render_object = requires(T obj, f64 delta_time) {
  { obj.on_render(delta_time) } -> std::same_as<void>;
};

template<typename F>
concept fixed_render_func = std::invocable<F, f64, f64>; // f(dt, alpha) -> void

template<typename F, u32 UPS>
concept fixed_update_func =
  std::invocable<F, u32> || std::invocable<F, std::integral_constant<u32, UPS>>; // f(ups) -> void

template<typename T>
concept fixed_render_object = requires(T obj, f64 delta_time, f64 alpha) {
  { obj.on_render(delta_time, alpha) } -> std::convertible_to<void>;
};

template<typename T, u32 UPS>
concept fixed_update_object = requires(T obj, u32 fixed_delta) {
  { obj.on_fixed_update(fixed_delta) } -> std::same_as<void>;
} || requires(T obj) {
  { obj.on_fixed_update(std::integral_constant<u32, UPS>{}) } -> std::same_as<void>;
};

template<typename T, u32 UPS>
concept fixed_loop_object = (fixed_render_func<T> && fixed_update_func<T, UPS>) ||
                            (fixed_render_object<T> && fixed_update_object<T, UPS>);

template<typename T>
concept delta_loop_object = delta_render_func<T> || delta_render_object<T>;

} // namespace meta

#ifndef SHOGLE_DISABLE_GLFW

template<::shogle::meta::delta_loop_object LoopObj>
void render_loop(glfw_win& win, gl_context& gl, const gl_clear_opts& clear, LoopObj&& obj) {
  NTF_ASSERT(win.context_type() == ::shogle::render_context_tag::opengl);
  using namespace std::literals;

  using clock = std::chrono::steady_clock;
  using duration = clock::duration;
  using time_point = std::chrono::time_point<clock, duration>;

  time_point last_time = clock::now();
  while (!win.should_close()) {
    const time_point start_time = clock::now();
    const auto elapsed_time = start_time - last_time;
    last_time = start_time;
    const f64 dt = (std::chrono::duration<f64>(elapsed_time) / 1s);

    win.poll_events();
    gl.start_frame(clear);
    if constexpr (::shogle::meta::delta_render_object<LoopObj>) {
      obj.on_render(dt);
    } else {
      obj(dt);
    }
    gl.end_frame();
    win.swap_buffers();
  }
};

template<u32 UPS, ::shogle::meta::fixed_loop_object<UPS> LoopObj>
void render_loop(glfw_win& win, gl_context& gl, const gl_clear_opts& clear, LoopObj&& obj) {
  NTF_ASSERT(win.context_type() == ::shogle::render_context_tag::opengl);
  using namespace std::literals;

  static constexpr std::chrono::duration<f64> fixed_elapsed_time =
    std::chrono::microseconds(1000000 / UPS);

  using clock = std::chrono::steady_clock;
  using duration = decltype(clock::duration{} + fixed_elapsed_time);
  using time_point = std::chrono::time_point<clock, duration>;

  time_point last_time = clock::now();
  duration lag = 0s;
  while (!win.should_close()) {
    const time_point start_time = clock::now();
    const auto elapsed_time = start_time - last_time;
    last_time = start_time;
    lag += elapsed_time;

    const f64 dt = (std::chrono::duration<f64>(elapsed_time) / 1s);
    const f64 alpha = (std::chrono::duration<f64>(lag) / fixed_elapsed_time);

    win.poll_events();

    gl.start_frame(clear);
    while (lag >= fixed_elapsed_time) {
      if constexpr (::shogle::meta::fixed_update_object<LoopObj, UPS>) {
        obj.on_fixed_update(std::integral_constant<u32, UPS>{});
      } else {
        obj(std::integral_constant<u32, UPS>{});
      }
      lag -= fixed_elapsed_time;
    }

    if constexpr (::shogle::meta::fixed_render_object<LoopObj>) {
      obj.on_render(dt, alpha);
    } else {
      obj(dt, alpha);
    }
    gl.end_frame();
    win.swap_buffers();
  }
}
#endif

} // namespace shogle
