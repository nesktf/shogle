#include <shogle/core.hpp>

#ifdef SHOGLE_ENABLE_IMGUI
#ifdef SHOGLE_ENABLE_GLFW
#include <imgui_impl_glfw.h>
#endif

#ifdef SHOGLE_ENABLE_OPENGL
#include <imgui_impl_opengl3.h>
#endif

#endif

#include <shogle/render/window.hpp>

#if defined(SHOGLE_ENABLE_GLFW) && defined(SHOGLE_ENABLE_OPENGL)
#include <shogle/render/gl/common.hpp>
static_assert(shogle::gl_provider_type<shogle::glfw_win>);
#endif

namespace shogle {

#ifdef SHOGLE_ENABLE_GLFW

#define WIN_LOG(level_, fmt_, ...) SHOGLE_RENDER_LOG(level_, "GLFW", fmt_, __VA_ARGS__)

struct glfw_win::window_data {
  window_data(GLFWwindow* win_, shogle::render_context_tag ctx_tag_) noexcept :
      win(win_), ctx_tag(ctx_tag_), on_viewport(), on_key_input(), on_cursor_pos(),
      on_cursor_enter(), on_scroll(), on_button_input(), on_char_input() {}

  ~window_data() noexcept {
    WIN_LOG(debug, "Window destroyed (ptr: {})", fmt::ptr(win));
    glfwDestroyWindow(win);
  }

  GLFWwindow* win;
  shogle::render_context_tag ctx_tag;
  viewport_fun on_viewport;
  key_input_fun on_key_input;
  cursor_pos_fun on_cursor_pos;
  cursor_enter_fun on_cursor_enter;
  scroll_fun on_scroll;
  button_input_fun on_button_input;
  char_input_fun on_char_input;
};

void glfw_win::fb_callback(GLFWwindow* win, int w, int h) {
  auto& data = *static_cast<glfw_win::window_data*>(glfwGetWindowUserPointer(win));
  if (data.on_viewport) {
    extent2d ext{.width = (u32)w, .height = (u32)h};
    std::invoke(data.on_viewport, data.win, ext);
  }
}

void glfw_win::key_callback(GLFWwindow* win, int key, int scan, int action, int mod) {
  auto& data = *static_cast<glfw_win::window_data*>(glfwGetWindowUserPointer(win));
  if (data.on_key_input) {
    const glfw_key_data keys{.key = (glfw_enum)key,
                             .scancode = (glfw_enum)scan,
                             .action = (glfw_enum)action,
                             .modifiers = (glfw_enum)mod};
    std::invoke(data.on_key_input, data.win, keys);
  }
}

void glfw_win::cursorp_callback(GLFWwindow* win, double xpos, double ypos) {
  auto& data = *static_cast<glfw_win::window_data*>(glfwGetWindowUserPointer(win));
  if (data.on_cursor_pos) {
    std::invoke(data.on_cursor_pos, data.win, (f64)xpos, (f64)ypos);
  }
}

void glfw_win::cursore_callback(GLFWwindow* win, int enters) {
  auto& data = *static_cast<glfw_win::window_data*>(glfwGetWindowUserPointer(win));
  if (data.on_cursor_enter) {
    std::invoke(data.on_cursor_enter, data.win, (bool)enters);
  }
}

void glfw_win::scroll_callback(GLFWwindow* win, double xoff, double yoff) {
  auto& data = *static_cast<glfw_win::window_data*>(glfwGetWindowUserPointer(win));
  if (data.on_scroll) {
    std::invoke(data.on_scroll, data.win, (f64)xoff, (f64)yoff);
  }
}

void glfw_win::char_callback(GLFWwindow* win, unsigned int codepoint) {
  auto& data = *static_cast<glfw_win::window_data*>(glfwGetWindowUserPointer(win));
  if (data.on_char_input) {
    std::invoke(data.on_char_input, data.win, (u32)codepoint);
  }
}

void glfw_win::button_callback(GLFWwindow* win, int button, int action, int mod) {
  auto& data = *static_cast<glfw_win::window_data*>(glfwGetWindowUserPointer(win));
  if (data.on_button_input) {
    const glfw_button_data butt{
      .button = (glfw_enum)button,
      .action = (glfw_enum)action,
      .modifiers = (glfw_enum)mod,
    };
    std::invoke(data.on_button_input, data.win, butt);
  }
}

glfw_gl_hints glfw_gl_hints::make_default(u32 ver_maj, u32 ver_min, u32 swap_interval) noexcept {
  return {
    .gl_ver = {.major = ver_maj, .minor = ver_min},
    .surface_buffer = SURFACE_BUFFER_D24US8,
    .window_alpha = ALPHA_DISABLE,
    .msaa_samples = 0,
    .swap_interval = swap_interval,
    .x11 = {},
  };
}

auto glfw_win::initialize_lib() -> glfw_lib {
  glfwInit();
  return {};
}

void glfw_win::glfw_lib::terminate() const noexcept {
  glfwTerminate();
}

glfw_win::glfw_win(create_t, window_data_ptr&& ctx) noexcept : _ctx(std::move(ctx)) {}

glfw_win::glfw_win(u32 width, u32 height, const char* name, const glfw_gl_hints& hints,
                   GLFWmonitor* monitor, GLFWwindow* share) :
    glfw_win(::shogle::glfw_win::create(width, height, name, hints, monitor, share).value()) {}

sv_expect<glfw_win> glfw_win::create(u32 width, u32 height, const char* title,
                                     const glfw_gl_hints& hints, GLFWmonitor* monitor,
                                     GLFWwindow* share) {
  if (!title) {
    return {unexpect, "No window title provided"};
  }
  if (!width || !height) {
    return {unexpect, "Invalid window extent"};
  }

  const auto round_pow2 = [](u32 x) -> u32 {
    x--;
    x |= x >> (1 << 0);
    x |= x >> (1 << 1);
    x |= x >> (1 << 2);
    x |= x >> (1 << 3);
    x |= x >> (1 << 4);
    x++;
    return x;
  };

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, hints.gl_ver.major);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, hints.gl_ver.minor);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, hints.window_alpha);
  if (!hints.window_alpha && hints.msaa_samples) {
    const u32 msaa = hints.msaa_samples > 64 ? 64 : round_pow2(hints.msaa_samples);
    glfwWindowHint(GLFW_SAMPLES, msaa);
  }

  static constexpr auto buffer_bits = std::to_array<std::pair<int, int>>({
    {24, 8},
    {24, 0},
    {32, 8},
    {32, 0},
    {0, 0},
  });
  const auto [depth_bits, stencil_bits] =
    buffer_bits[(size_t)hints.surface_buffer > buffer_bits.size()
                  ? glfw_gl_hints::SURFACE_BUFFER_D24US8
                  : (size_t)hints.surface_buffer];
  glfwWindowHint(GLFW_DEPTH_BITS, depth_bits);
  glfwWindowHint(GLFW_STENCIL_BITS, stencil_bits);

  if (hints.x11 && hints.x11->class_name) {
    glfwWindowHintString(GLFW_X11_CLASS_NAME, hints.x11->class_name);
  } else {
    glfwWindowHintString(GLFW_X11_CLASS_NAME, title);
  }
  if (hints.x11 && hints.x11->instance_name) {
    glfwWindowHintString(GLFW_X11_INSTANCE_NAME, hints.x11->instance_name);
  } else {
    glfwWindowHintString(GLFW_X11_INSTANCE_NAME, title);
  }

  GLFWwindow* win = glfwCreateWindow(width, height, title, monitor, share);
  if (!win) {
    const char* err;
    glfwGetError(&err);
    WIN_LOG(error, "Failed to create window: {}", err);
    return {unexpect, err};
  }

  auto* data_ptr = new glfw_win::window_data(win, render_context_tag::opengl);

  glfwSetFramebufferSizeCallback(win, fb_callback);
  glfwSetKeyCallback(win, key_callback);
  glfwSetCursorPosCallback(win, cursorp_callback);
  glfwSetCursorEnterCallback(win, cursore_callback);
  glfwSetScrollCallback(win, scroll_callback);
  glfwSetCharCallback(win, char_callback);
  glfwSetMouseButtonCallback(win, button_callback);
  glfwSetWindowUserPointer(win, data_ptr);

  glfwMakeContextCurrent(win);
  WIN_LOG(verbose, "Current OpenGL window context: {}", fmt::ptr(win));
  glfwSwapInterval(hints.swap_interval);

  WIN_LOG(debug, "OpenGL window created (w: {}, h: {}, title: \"{}\", ptr: {})", width, height,
          title, fmt::ptr(win));

  return {in_place, create_t{}, window_data_ptr(data_ptr)};
}

void glfw_win::window_deleter::operator()(window_data* data) noexcept {
  delete data;
}

void glfw_win::destroy() noexcept {
  _ctx.reset();
}

GLFWwindow* glfw_win::get() const {
  SHOGLE_ASSERT(_ctx);
  return _ctx->win;
}

bool glfw_win::should_close() const {
  SHOGLE_ASSERT(_ctx);
  return glfwWindowShouldClose(_ctx->win);
}

void glfw_win::close() const {
  SHOGLE_ASSERT(_ctx);
  glfwSetWindowShouldClose(_ctx->win, 1);
}

void glfw_win::poll_events() const {
  glfwPollEvents();
}

void glfw_win::set_title(const char* title) const {
  SHOGLE_ASSERT(_ctx);
  glfwSetWindowTitle(_ctx->win, title);
}

extent2d glfw_win::window_extent() const noexcept {
  if (SHOGLE_UNLIKELY(!_ctx)) {
    return {.width = 0, .height = 0};
  }
  int w, h;
  glfwGetWindowSize(_ctx->win, &w, &h);
  return {.width = (u32)w, .height = (u32)h};
}

extent2d glfw_win::surface_extent() const noexcept {
  if (SHOGLE_UNLIKELY(!_ctx)) {
    return {.width = 0, .height = 0};
  }
  int w, h;
  glfwGetFramebufferSize(_ctx->win, &w, &h);
  return {.width = (u32)w, .height = (u32)h};
}

void glfw_win::set_swap_interval(u32 interval) const noexcept {
  if (_ctx && _ctx->ctx_tag == render_context_tag::opengl) {
    glfwSwapInterval(interval);
  }
}

void glfw_win::set_attrib(glfw_enum attrib, glfw_enum value) const {
  SHOGLE_ASSERT(_ctx);
  glfwSetWindowAttrib(_ctx->win, (int)attrib, (int)value);
}

glfw_enum glfw_win::poll_key(glfw_enum key) const {
  SHOGLE_ASSERT(_ctx);
  return (glfw_enum)glfwGetKey(_ctx->win, key);
}

shogle::render_context_tag glfw_win::context_type() const noexcept {
  if (SHOGLE_UNLIKELY(!_ctx)) {
    return ::shogle::render_context_tag::none;
  }
  return _ctx->ctx_tag;
}

void glfw_win::swap_buffers() noexcept {
  if (SHOGLE_UNLIKELY(!_ctx)) {
    return;
  }
  glfwSwapBuffers(_ctx->win);
}

void* glfw_win::gl_get_proc(const char* name) const noexcept {
  return (reinterpret_cast<void* (*)(const char*)>(glfwGetProcAddress))(name);
}

/*
u32 glfw_win::vk_surface_extensions(scratch_vec<const char*>& extensions) {
  SHOGLE_UNUSED(extensions);
  SHOGLE_ASSERT(false, "TODO");
  return 0;
}

bool glfw_win::vk_create_surface(vkdefs::VkInstance vk, vkdefs::VkSurfaceKHR& surface,
                                 vkdefs::VkAllocationCallbacksPtr vkalloc) noexcept {
  SHOGLE_UNUSED(vk);
  SHOGLE_UNUSED(surface);
  SHOGLE_UNUSED(vkalloc);
  SHOGLE_ASSERT(false, "TODO");
  return true;
}
*/

glfw_win& glfw_win::set_viewport_callback(viewport_fun func) {
  SHOGLE_ASSERT(_ctx);
  _ctx->on_viewport = std::move(func);
  return *this;
}

void glfw_win::remove_viewport_callback() {
  SHOGLE_ASSERT(_ctx);
  _ctx->on_viewport = {};
}

glfw_win& glfw_win::set_key_input_callback(key_input_fun func) {
  SHOGLE_ASSERT(_ctx);
  _ctx->on_key_input = std::move(func);
  return *this;
}

void glfw_win::remove_key_input_callback() {
  SHOGLE_ASSERT(_ctx);
  _ctx->on_key_input = {};
}

glfw_win& glfw_win::set_cursor_pos_callback(cursor_pos_fun func) {
  SHOGLE_ASSERT(_ctx);
  _ctx->on_cursor_pos = std::move(func);
  return *this;
}

void glfw_win::remove_cursor_pos_callback() {
  SHOGLE_ASSERT(_ctx);
  _ctx->on_cursor_pos = {};
}

glfw_win& glfw_win::set_cursor_enter_callback(cursor_enter_fun func) {
  SHOGLE_ASSERT(_ctx);
  _ctx->on_cursor_enter = std::move(func);
  return *this;
}

void glfw_win::remove_cursor_enter_callback() {
  SHOGLE_ASSERT(_ctx);
  _ctx->on_cursor_enter = {};
}

glfw_win& glfw_win::set_scroll_callback(scroll_fun func) {
  SHOGLE_ASSERT(_ctx);
  _ctx->on_scroll = std::move(func);
  return *this;
}

void glfw_win::remove_scroll_callback() {
  SHOGLE_ASSERT(_ctx);
  _ctx->on_scroll = {};
}

glfw_win& glfw_win::set_button_input_callback(button_input_fun func) {
  SHOGLE_ASSERT(_ctx);
  _ctx->on_button_input = std::move(func);
  return *this;
}

void glfw_win::remove_mouse_input_callback() {
  SHOGLE_ASSERT(_ctx);
  _ctx->on_button_input = {};
}

glfw_win& glfw_win::set_char_input_callback(char_input_fun func) {
  SHOGLE_ASSERT(_ctx);
  _ctx->on_char_input = std::move(func);
  return *this;
}

void glfw_win::remove_char_input_callback() {
  SHOGLE_ASSERT(_ctx);
  _ctx->on_char_input = {};
}

#ifdef SHOGLE_ENABLE_IMGUI
glfw_imgui::glfw_imgui(create_t, GLFWwindow* win, shogle::render_context_tag ctx_type) noexcept :
    _win(win), _ctx_type(ctx_type) {}

glfw_imgui::glfw_imgui(GLFWwindow* win, shogle::render_context_tag ctx_type,
                       ImGuiConfigFlags flags, callback_flags callbacks) :
    glfw_imgui(::shogle::glfw_imgui::create(win, ctx_type, flags, callbacks).value()) {}

glfw_imgui::glfw_imgui(const glfw_win& win, ImGuiConfigFlags flags, callback_flags callbacks) :
    glfw_imgui(::shogle::glfw_imgui::create(win, flags, callbacks).value()) {}

sv_expect<glfw_imgui> glfw_imgui::create(const glfw_win& win, ImGuiConfigFlags flags,
                                         callback_flags callbacks) noexcept {
  return ::shogle::glfw_imgui::create(win.get(), win.context_type(), flags, callbacks);
}

sv_expect<glfw_imgui> glfw_imgui::create(GLFWwindow* win, shogle::render_context_tag ctx_type,
                                         ImGuiConfigFlags flags,
                                         callback_flags callbacks) noexcept {
  if (!win) {
    return {unexpect, "Invalid GLFW window"};
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags = flags;
  ImGui::StyleColorsDark();
  switch (ctx_type) {
    case render_context_tag::opengl: {
      ImGui_ImplGlfw_InitForOpenGL(win, (bool)callbacks);
      ImGui_ImplOpenGL3_Init("#version 150");
    } break;
    default:
      SHOGLE_ASSERT(false, "TODO");
  }

  return {in_place, create_t{}, win, ctx_type};
}

void glfw_imgui::start_frame() {
  SHOGLE_ASSERT(_win);
  switch (_ctx_type) {
    case render_context_tag::opengl: {
      ImGui_ImplGlfw_NewFrame();
      ImGui_ImplOpenGL3_NewFrame();
    } break;
    default:
      SHOGLE_ASSERT(false, "TODO");
  }
  ImGui::NewFrame();
}

void glfw_imgui::end_frame() {
  SHOGLE_ASSERT(_win);
  ImGui::Render();
  auto* draw_data = ImGui::GetDrawData();
  switch (_ctx_type) {
    case render_context_tag::opengl: {
      ImGui_ImplOpenGL3_RenderDrawData(draw_data);
    } break;
    default:
      SHOGLE_ASSERT(false, "TODO");
  }
}

void glfw_imgui::destroy() noexcept {
  if (_win) {
    switch (_ctx_type) {
      case render_context_tag::opengl: {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
      } break;
      default:
        SHOGLE_ASSERT(false, "TODO");
    }
    ImGui::DestroyContext();
    _win = nullptr;
  }
}

glfw_imgui::~glfw_imgui() noexcept {
  destroy();
}

glfw_imgui::glfw_imgui(glfw_imgui&& other) noexcept :
    _win(other._win), _ctx_type(other._ctx_type) {
  other._win = nullptr;
}

glfw_imgui& glfw_imgui::operator=(glfw_imgui&& other) noexcept {
  destroy();

  _win = other._win;
  _ctx_type = other._ctx_type;

  other._win = nullptr;

  return *this;
}
#endif

#endif

} // namespace shogle
