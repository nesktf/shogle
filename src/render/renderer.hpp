#pragma once

#include "./context.hpp"
#include "./window.hpp"

namespace ntf {

class r_context_view {
public:
  r_context_view(r_context ctx) noexcept :
    _ctx{ctx} {}

public:
  r_context handle() const { return _ctx; }

  void start_frame() const {
    r_start_frame(_ctx);
  }
  void end_frame() const {
    r_end_frame(_ctx);
  }
  void device_wait() const {
    r_device_wait(_ctx);
  }
  void submit_command(const r_draw_command& cmd) const {
    r_submit_command(_ctx, cmd);
  }

protected:
  r_context _ctx;
};

class renderer_context : public r_context_view {
private:
  struct deleter_t {
    void operator()(r_context ctx) {
      r_destroy_context(ctx);
    }
  };
  using uptr_type = std::unique_ptr<r_context_, deleter_t>;

public:
  explicit renderer_context(r_context ctx) noexcept :
    r_context_view{ctx},
    _handle{ctx} {}

public:
  static auto create(
    const r_context_params& params
  ) noexcept -> r_expected<renderer_context>
  {
    return r_create_context(params)
      .transform([](r_context ctx) -> renderer_context {
        return renderer_context{ctx};
      });
  }

private:
  uptr_type _handle;
};

template<typename F>
concept delta_time_func = std::invocable<F, float64>; // f(dt) -> void

template<typename F>
concept fixed_render_func = std::invocable<F, float64, float64>; // f(dt, alpha) -> void

template<typename F>
concept fixed_update_func = std::invocable<F, uint32>; // f(ups) -> void

template<typename T>
concept has_render_member = requires(T t) {
  { t.on_render(float64{}) } -> std::convertible_to<void>;
};

template<typename T>
concept has_fixed_render_member = requires(T t) {
  { t.on_render(float64{}, float64{}) } -> std::convertible_to<void>;
};

template<typename T>
concept has_fixed_update_member = requires(T t) {
  { t.on_fixed_update(uint32{}) } -> std::convertible_to<void>;
};

template<typename T>
concept fixed_loop_object = 
  (fixed_render_func<T> && fixed_update_func<T>) ||
  (has_fixed_render_member<T> && has_fixed_update_member<T>);

template<typename T>
concept nonfixed_loop_object = delta_time_func<T> || has_render_member<T>;


template<nonfixed_loop_object LoopObj>
void shogle_render_loop(renderer_window& window, r_context_view ctx, LoopObj&& obj) {
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

    float64 dt {std::chrono::duration<float64>{elapsed_time}/1s};

    window.poll_events();

    ctx.start_frame();
    if constexpr (has_render_member<LoopObj>) {
      obj.on_render(dt);
    } else {
      obj(dt);
    }
    ctx.end_frame();
  }
  ctx.device_wait();

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Main loop exit");
}

template<fixed_loop_object LoopObj>
void shogle_render_loop(renderer_window& window, r_context_view ctx, const uint32& ups, LoopObj&& obj) {
  using namespace std::literals;

  auto fixed_elapsed_time =
    std::chrono::duration<float64>{std::chrono::microseconds{1000000/ups}};

  using clock = std::chrono::steady_clock;
  using duration = decltype(clock::duration{} + fixed_elapsed_time);
  using time_point = std::chrono::time_point<clock, duration>;

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Fixed main loop started at {} ups", ups);

  time_point last_time = clock::now();
  duration lag = 0s;
  while (!window.should_close()) {
    fixed_elapsed_time = std::chrono::duration<float64>{std::chrono::microseconds{1000000/ups}};
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
    last_time = start_time;
    lag += elapsed_time;

    float64 dt {std::chrono::duration<float64>{elapsed_time}/1s};
    float64 alpha {std::chrono::duration<float64>{lag}/fixed_elapsed_time};

    window.poll_events();

    ctx.start_frame();

    while (lag >= fixed_elapsed_time) {
      if constexpr (has_fixed_update_member<LoopObj>) {
        obj.on_fixed_update(ups);
      } else {
        obj(ups);
      }
      lag -= fixed_elapsed_time;
    }

    if constexpr (has_fixed_render_member<LoopObj>) {
      obj.on_render(dt, alpha);
    } else {
      obj(dt, alpha);
    }

    ctx.end_frame();
  }
  ctx.device_wait();

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Main loop exit");
}

template<fixed_render_func RFunc, fixed_update_func UFunc>
void shogle_render_loop(renderer_window& window, r_context_view ctx, const uint32& ups,
                        RFunc&& render, UFunc&& fixed_update) {
  using namespace std::literals;

  auto fixed_elapsed_time =
    std::chrono::duration<float64>{std::chrono::microseconds{1000000/ups}};

  using clock = std::chrono::steady_clock;
  using duration = decltype(clock::duration{} + fixed_elapsed_time);
  using time_point = std::chrono::time_point<clock, duration>;

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Fixed main loop started at {} ups", ups);

  time_point last_time = clock::now();
  duration lag = 0s;
  while (!window.should_close()) {
    fixed_elapsed_time = std::chrono::duration<float64>{std::chrono::microseconds{1000000/ups}};
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
    last_time = start_time;
    lag += elapsed_time;

    float64 dt {std::chrono::duration<float64>{elapsed_time}/1s};
    float64 alpha {std::chrono::duration<float64>{lag}/fixed_elapsed_time};

    window.poll_events();

    ctx.start_frame();

    while (lag >= fixed_elapsed_time) {
      fixed_update(ups);
      lag -= fixed_elapsed_time;
    }

    render(dt, alpha);

    ctx.end_frame();
  }
  ctx.device_wait();

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Main loop exit");
}

} // namespace ntf
