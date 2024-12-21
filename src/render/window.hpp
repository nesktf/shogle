#pragma once

#include "render/render.hpp"
#include "stl/function.hpp"

#include "keys.hpp"

#if SHOGLE_ENABLE_IMGUI
#include <imgui_impl_glfw.h>
#endif

#include <chrono>

namespace ntf {

struct r_window_hints {
  std::string_view x11_class_name{};
  std::string_view x11_instance_name{};
};

class r_window {
public:
  using viewport_fun = ntf::inplace_function<void(uint32,uint32)>;
  using key_fun = ntf::inplace_function<void(keycode,scancode,keystate,keymod)>;
  using cursor_fun = ntf::inplace_function<void(float64,float64)>;
  using scroll_fun = ntf::inplace_function<void(float64,float64)>;

public:
#if SHOGLE_USE_GLFW
  template<typename RenderCtx>
  r_window(RenderCtx& ctx, uint32 w, uint32 h, std::string_view title, r_window_hints hints = {}) {
    if (!glfwInit()) {
      SHOGLE_LOG(error, "[ntf::r_window] Failed to initialize GLFW");
      return;
    }

    constexpr r_api ctx_api = RenderCtx::RENDER_API;
    if constexpr (ctx_api == r_api::opengl) {
      const auto [ver_maj, ver_min] = ctx.version();
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, ver_maj);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, ver_min);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    } else if constexpr (ctx_api == r_api::vulkan) {
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }

    if (!hints.x11_class_name.empty()) {
      glfwWindowHintString(GLFW_X11_CLASS_NAME, hints.x11_class_name.data());
    }

    if (!hints.x11_instance_name.empty()) {
      glfwWindowHintString(GLFW_X11_INSTANCE_NAME, hints.x11_instance_name.data());
    }

    GLFWwindow* win = glfwCreateWindow(w, h, title.data(), nullptr, nullptr);
    if (!win) {
      glfwTerminate();
      SHOGLE_LOG(error, "[ntf::r_window] Failed to create window");
      return;
    }

    if constexpr (ctx_api == r_api::opengl) {
      glfwMakeContextCurrent(win);
    }

#if SHOGLE_ENABLE_IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();

    if constexpr (ctx_api == r_api::opengl) {
      ImGui_ImplGlfw_InitForOpenGL(win, true);
    } else if (ctx_api == r_api::vulkan) {
      ImGui_ImplGlfw_InitForVulkan(win, true);
    } else {
      ImGui_ImplGlfw_InitForOther(win, true);
    }
#endif
    if constexpr (ctx_api == r_api::opengl) {
      if (!ctx.init(glfwGetProcAddress, [win]() { glfwSwapBuffers(win); })) {
        glfwDestroyWindow(win);
        glfwTerminate();
        SHOGLE_LOG(error, "[ntf::r_window] Failed to initialize OpenGL context");
        return;
      }
    }

    glfwSetWindowUserPointer(win, this);

    glfwSetFramebufferSizeCallback(win, r_window::fb_callback);
    glfwSetKeyCallback(win, r_window::key_callback);
    glfwSetCursorPosCallback(win, r_window::cursor_callback);
    glfwSetScrollCallback(win, r_window::scroll_callback);

    _handle = win;
    _ctx = reinterpret_cast<void*>(&ctx);
    _ctx_api = ctx_api;
  }
#endif

public:
  void viewport_event(viewport_fun callback);
  void key_event(key_fun callback);
  void cursor_event(cursor_fun callback);
  void scroll_event(scroll_fun callback);

  void title(std::string_view title);

  void close();

  void poll_events();

public:
  bool should_close() const;
  bool poll_key(keycode key, keystate state) const;
  uvec2 win_size() const;
  uvec2 fb_size() const;

  bool valid() const { return _handle != nullptr; }
  explicit operator bool() const { return valid(); }

private:
#if SHOGLE_USE_GLFW
  static void fb_callback(GLFWwindow* handle, int w, int h);
  static void key_callback(GLFWwindow* handle, int code, int scan, int state, int mod);
  static void cursor_callback(GLFWwindow* handle, double xpos, double ypos);
  static void scroll_callback(GLFWwindow* handle, double xoff, double yoff);
#endif

private:
#if SHOGLE_USE_GLFW
  GLFWwindow* _handle{nullptr};
#endif
  void* _ctx{nullptr};
  viewport_fun _viewport_event;
  key_fun _key_event;
  cursor_fun _cursor_event;
  scroll_fun _scroll_event;
  r_api _ctx_api{r_api::none};

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(r_window);
};


template<typename F>
concept delta_time_func = std::invocable<F, float64>; // f(dt) -> void

template<typename F>
concept fixed_render_func = std::invocable<F, float64, float64>; // f(dt, alpha) -> void

template<typename F>
concept fixed_update_func = std::invocable<F, uint32>; // f(ups) -> void


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
  { t.on_fixed_update(uint32{}) } -> std::convertible_to<void>;
};


template<typename T>
concept fixed_loop_object = 
  (fixed_render_func<T> && fixed_update_func<T>) ||
  (has_fixed_render_member<T> && has_fixed_update_member<T>);

template<typename T>
concept nonfixed_loop_object = delta_time_func<T> || has_render_member<T>;


template<render_context_object RenderCtx, nonfixed_loop_object LoopObj>
void shogle_render_loop(RenderCtx& ctx, r_window& window, LoopObj&& obj) {
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

template<render_context_object RenderCtx, fixed_loop_object LoopObj>
void shogle_render_loop(RenderCtx& ctx, r_window& window, uint32 ups, LoopObj&& obj) {
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

template<render_context_object RenderCtx, fixed_render_func RFunc, fixed_update_func UFunc>
void shogle_render_loop(RenderCtx& ctx, r_window& window, uint32 ups,
                        RFunc&& render, UFunc&& fixed_update) {
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
