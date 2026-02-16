#pragma once

#include <shogle/render/common.hpp>

#include <shogle/util/expected.hpp>

#ifdef SHOGLE_ENABLE_GLFW
#include <GLFW/glfw3.h>
#endif

#ifdef SHOGLE_ENABLE_IMGUI
#ifndef IMGUI_API
#include <shogle/extern/imgui.h>
#endif
#endif

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
using glfw_enum = int;

struct glfw_x11_hints {
  const char* class_name;
  const char* instance_name;
};

struct glfw_gl_version {
  u32 major;
  u32 minor;
};

struct glfw_gl_hints {
public:
  enum alpha_mode : glfw_enum {
    ALPHA_DISABLE = 0,
    ALPHA_ENABLE = 1,
  };

  enum surface_buffer : glfw_enum {
    SURFACE_BUFFER_D24US8 = 0,
    SURFACE_BUFFER_D24,
    SURFACE_BUFFER_D32FS8,
    SURFACE_BUFFER_D32F,
    SURFACE_BUFFER_NONE,
  };

public:
  static glfw_gl_hints make_default(u32 ver_maj, u32 ver_min, u32 swap_interval = 1) noexcept;

public:
  glfw_gl_version gl_ver;
  surface_buffer surface_buffer;
  alpha_mode window_alpha;
  u32 msaa_samples;
  u32 swap_interval;
  optional<glfw_x11_hints> x11;
};

struct glfw_key_data {
  glfw_enum key;
  glfw_enum scancode;
  glfw_enum action;
  glfw_enum modifiers;
};

struct glfw_button_data {
  glfw_enum button;
  glfw_enum action;
  glfw_enum modifiers;
};

class glfw_win {
private:
  template<typename Signature>
  using callback_type = std::function<Signature>;

  using viewport_fun = callback_type<void(GLFWwindow*, extent2d)>;
  using key_input_fun = callback_type<void(GLFWwindow*, const glfw_key_data&)>;
  using cursor_pos_fun = callback_type<void(GLFWwindow*, f64, f64)>;
  using cursor_enter_fun = callback_type<void(GLFWwindow*, bool)>;
  using scroll_fun = callback_type<void(GLFWwindow*, f64, f64)>;
  using button_input_fun = callback_type<void(GLFWwindow*, const glfw_button_data&)>;
  using char_input_fun = callback_type<void(GLFWwindow*, u32)>;

  static void fb_callback(GLFWwindow* win, int w, int h);
  static void key_callback(GLFWwindow* win, int key, int scan, int action, int mod);
  static void cursorp_callback(GLFWwindow* win, double xpos, double ypos);
  static void cursore_callback(GLFWwindow* win, int enters);
  static void scroll_callback(GLFWwindow* win, double xoff, double yoff);
  static void char_callback(GLFWwindow* win, unsigned int codepoint);
  static void button_callback(GLFWwindow* win, int button, int action, int mod);

  struct glfw_lib {
    ~glfw_lib() noexcept { terminate(); }

    void terminate() const noexcept;
  };

  struct create_t {};

public:
  struct window_data;

  struct window_deleter {
    void operator()(window_data* data) noexcept;
  };

  using window_data_ptr = std::unique_ptr<window_data, window_deleter>;

public:
  glfw_win(create_t, window_data_ptr&& ctx) noexcept;

  glfw_win(u32 width, u32 height, const char* title, const glfw_gl_hints& hints,
           GLFWmonitor* monitor = nullptr, GLFWwindow* share = nullptr);

public:
  static glfw_lib initialize_lib();

  static sv_expect<glfw_win> create(u32 width, u32 height, const char* title,
                                    const glfw_gl_hints& hints, GLFWmonitor* monitor = nullptr,
                                    GLFWwindow* share = nullptr);

public:
  void destroy() noexcept;
  GLFWwindow* get() const;
  bool should_close() const;
  void close() const;
  void poll_events() const;
  void set_title(const char* title) const;
  extent2d window_extent() const noexcept;
  void set_swap_interval(u32 interval) const noexcept;
  void set_attrib(glfw_enum attrib, glfw_enum value) const;
  glfw_enum poll_key(glfw_enum key) const;
  shogle::render_context_tag context_type() const noexcept;
  void swap_buffers() noexcept;

public:
  void* gl_get_proc(const char* name) const noexcept;
  extent2d surface_extent() const noexcept;
  // u32 vk_surface_extensions(scratch_vec<const char*>& extensions) override;
  // bool vk_create_surface(vkdefs::VkInstance vk, vkdefs::VkSurfaceKHR& surface,
  //                        vkdefs::VkAllocationCallbacksPtr vkalloc) noexcept override;

public:
  glfw_win& set_viewport_callback(viewport_fun func);
  void remove_viewport_callback();

  glfw_win& set_key_input_callback(key_input_fun func);
  void remove_key_input_callback();

  glfw_win& set_cursor_pos_callback(cursor_pos_fun func);
  void remove_cursor_pos_callback();

  glfw_win& set_cursor_enter_callback(cursor_enter_fun func);
  void remove_cursor_enter_callback();

  glfw_win& set_scroll_callback(scroll_fun func);
  void remove_scroll_callback();

  glfw_win& set_button_input_callback(button_input_fun func);
  void remove_mouse_input_callback();

  glfw_win& set_char_input_callback(char_input_fun func);
  void remove_char_input_callback();

public:
  operator GLFWwindow*() const { return get(); }

private:
  window_data_ptr _ctx;
};

template<::shogle::meta::delta_loop_object LoopObj>
void render_loop(glfw_win& win, LoopObj&& obj) {
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
    if constexpr (::shogle::meta::delta_render_object<LoopObj>) {
      obj.on_render(dt);
    } else {
      obj(dt);
    }
    win.swap_buffers();
  }
};

template<u32 UPS, ::shogle::meta::fixed_loop_object<UPS> LoopObj>
void render_loop(glfw_win& win, LoopObj&& obj) {
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
    win.swap_buffers();
  }
}

#ifdef SHOGLE_ENABLE_IMGUI
class glfw_imgui {
public:
  static constexpr ImGuiConfigFlags DEFAULT_FLAGS =
    ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;

  enum callback_flags {
    CALLBACKS_DONT_INSTALL,
    CALLBACKS_INSTALL = 1,
  };

private:
  struct create_t {};

public:
  glfw_imgui(create_t, GLFWwindow* win, shogle::render_context_tag ctx_type) noexcept;

  glfw_imgui(GLFWwindow* win, shogle::render_context_tag ctx_type,
             ImGuiConfigFlags flags = DEFAULT_FLAGS, callback_flags callbacks = CALLBACKS_INSTALL);

  glfw_imgui(const glfw_win& win, ImGuiConfigFlags flags = DEFAULT_FLAGS,
             callback_flags callbacks = CALLBACKS_INSTALL);

  glfw_imgui(const glfw_imgui&) = delete;
  glfw_imgui(glfw_imgui&& other) noexcept;
  ~glfw_imgui() noexcept;

public:
  static sv_expect<glfw_imgui> create(const glfw_win& win, ImGuiConfigFlags flags = DEFAULT_FLAGS,
                                      callback_flags callbacks = CALLBACKS_INSTALL) noexcept;

  static sv_expect<glfw_imgui> create(GLFWwindow* win, shogle::render_context_tag ctx_type,
                                      ImGuiConfigFlags flags = DEFAULT_FLAGS,
                                      callback_flags callbacks = CALLBACKS_INSTALL) noexcept;
  void destroy() noexcept;

public:
  void start_frame();
  void end_frame();

  template<typename F>
  requires(std::invocable<F>)
  void scoped_frame(F&& func) {
    start_frame();
    std::invoke(func);
    end_frame();
  }

public:
  glfw_imgui& operator=(const glfw_imgui&) = delete;
  glfw_imgui& operator=(glfw_imgui&&) noexcept;

private:
  GLFWwindow* _win;
  ::shogle::render_context_tag _ctx_type;
};
#endif

#endif

} // namespace shogle
