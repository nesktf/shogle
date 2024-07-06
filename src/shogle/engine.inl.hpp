#define SHOGLE_ENGINE_INL_HPP
#include <shogle/engine.hpp>
#undef SHOGLE_ENGINE_INL_HPP

#include <shogle/core/log.hpp>

#include <thread>
#include <chrono>

namespace ntf::shogle {

template<fixrenderfunc RFunc, updatefunc FUFunc, updatefunc UFunc>
void engine_main_loop(uint ups, RFunc&& render, FUFunc&& fixed_update, UFunc&& update) {
  auto handle = __engine_window_handle();
  auto fixed_elapsed_time = std::chrono::duration<double>{std::chrono::microseconds{1000000/ups}};

  using namespace std::literals;
  using clock = std::chrono::steady_clock;
  using duration = decltype(clock::duration{} + fixed_elapsed_time);
  using time_point = std::chrono::time_point<clock, duration>;

  log::debug("[shogle::engine] Starting fixed main loop at {} ups", ups);

  time_point last_time = clock::now();
  duration lag = 0s;
  while (!glfwWindowShouldClose(handle)) {
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
    last_time = start_time;
    lag += elapsed_time;

    double fixed_dt {std::chrono::duration<double>{fixed_elapsed_time}/1s};
    double dt {std::chrono::duration<double>{elapsed_time}/1s};
    double alpha {std::chrono::duration<double>{lag}/fixed_elapsed_time};

    glfwPollEvents();

    while (lag >= fixed_elapsed_time) {
      fixed_update(fixed_dt);
      lag -= fixed_elapsed_time;
    }
    update(dt);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    render(dt, fixed_dt, alpha);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(handle);
  }

  log::debug("[shogle::engine] Main loop exited");
}

template<renderfunc RFunc, updatefunc UFunc>
void engine_main_loop(RFunc&& render, UFunc&& update) {
  auto handle = __engine_window_handle();

  using namespace std::literals;
  using clock = std::chrono::steady_clock;
  using duration = clock::duration;
  using time_point = std::chrono::time_point<clock, duration>;

  log::debug("[shogle::engine] Starting non-fixed main loop");

  time_point last_time = clock::now();
  while (!glfwWindowShouldClose(handle)) {
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
    last_time = start_time;

    double dt {std::chrono::duration<double>{elapsed_time}/1s};

    glfwPollEvents();

    update(dt);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    render(dt);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(handle);
  }

  log::debug("[shogle::engine] Main loop exited");
}

} // namespace ntf::shogle
