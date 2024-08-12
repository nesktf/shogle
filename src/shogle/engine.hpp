#pragma once

#include <shogle/core/log.hpp>

#include <shogle/render/render.hpp>

#include <shogle/keys.hpp>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <functional>
#include <thread>
#include <chrono>

namespace ntf {

namespace impl {

GLFWwindow* glfw_window_handle();

} // namespace impl

void engine_init(size_t width, size_t height, std::string_view title);
void engine_destroy();

void engine_viewport_event(std::function<void(size_t,size_t)> fun);
void engine_key_event(std::function<void(keycode,scancode,keystate,keymod)> fun);
void engine_cursor_event(std::function<void(double,double)> fun);
void engine_scroll_event(std::function<void(double,double)> fun);

bool engine_poll_key(keycode key);
ivec2 engine_window_size();
void engine_use_vsync(bool flag);
void engine_set_title(std::string_view title);
void engine_close_window();

template<typename F>
concept fixrenderfunc = std::invocable<F, double, double>;

template<typename F>
concept renderfunc = std::invocable<F>;

template<typename F>
concept updatefunc = std::invocable<F, double>;

template<typename F>
concept fupdatefunc = std::invocable<F>;

template<fixrenderfunc RFunc, fupdatefunc FUFunc>
void engine_main_loop(uint ups, RFunc&& render, FUFunc&& fixed_update) {
  auto handle = impl::glfw_window_handle();
  auto fixed_elapsed_time = std::chrono::duration<double>{std::chrono::microseconds{1000000/ups}};

  using namespace std::literals;
  using clock = std::chrono::steady_clock;
  using duration = decltype(clock::duration{} + fixed_elapsed_time);
  using time_point = std::chrono::time_point<clock, duration>;

  log::debug("[ntf::engine] Starting fixed main loop at {} ups", ups);

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
      fixed_update();
      lag -= fixed_elapsed_time;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    render(dt, alpha);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(handle);
  }

  log::debug("[ntf::engine] Main loop exited");
}

template<renderfunc RFunc, updatefunc UFunc>
void engine_main_loop(RFunc&& render, UFunc&& update) {
  auto handle = impl::glfw_window_handle();

  using namespace std::literals;
  using clock = std::chrono::steady_clock;
  using duration = clock::duration;
  using time_point = std::chrono::time_point<clock, duration>;

  log::debug("[ntf::engine] Starting non-fixed main loop");

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

  log::debug("[ntf::engine] Main loop exited");
}

} // namespace ntf
