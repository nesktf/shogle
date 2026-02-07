#pragma once

#include <shogle/render/common.hpp>

#ifndef SHOGLE_DISABLE_GLFW
#include <GLFW/glfw3.h>
#endif

#ifdef SHOGLE_ENABLE_IMGUI
#include <imgui.h>
#endif

namespace shogle {

#ifndef SHOGLE_DISABLE_GLFW
using glfw_enum = int;

struct glfw_x11_hints {
  const char* class_name;
  const char* instance_name;
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
  shogle::gl_version gl_ver;
  surface_buffer surface_buffer;
  alpha_mode window_alpha;
  u32 msaa_samples;
  u32 swap_interval;
  ntf::optional<glfw_x11_hints> x11;
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

class glfw_win : public gl_surface_provider, public vk_surface_provider {
private:
  template<typename Signature>
  using callback_type = ntf::inplace_function<Signature, 8 * sizeof(void*)>;

  using viewport_fun = callback_type<void(glfw_win&, extent2d)>;
  using key_input_fun = callback_type<void(glfw_win&, const glfw_key_data&)>;
  using cursor_pos_fun = callback_type<void(glfw_win&, f64, f64)>;
  using cursor_enter_fun = callback_type<void(glfw_win&, bool)>;
  using scroll_fun = callback_type<void(glfw_win&, f64, f64)>;
  using button_input_fun = callback_type<void(glfw_win&, const glfw_button_data&)>;
  using char_input_fun = callback_type<void(glfw_win&, u32)>;

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
  extent2d surface_extent() const noexcept;
  void set_swap_interval(u32 interval) const noexcept;
  void set_attrib(glfw_enum attrib, glfw_enum value) const;
  glfw_enum poll_key(glfw_enum key) const;
  shogle::render_context_tag context_type() const noexcept;
  void swap_buffers() noexcept;

public:
  PFN_glGetProcAddress gl_proc_loader() noexcept override;
  extent2d gl_surface_extent() const noexcept override;

  extent2d vk_surface_extent() const noexcept override;
  u32 vk_surface_extensions(scratch_vec<const char*>& extensions) override;
  bool vk_create_surface(vkdefs::VkInstance vk, vkdefs::VkSurfaceKHR& surface,
                         vkdefs::VkAllocationCallbacksPtr vkalloc) noexcept override;

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
