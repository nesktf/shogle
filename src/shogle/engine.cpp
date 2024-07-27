#include <shogle/engine.hpp>

#include <shogle/core/log.hpp>

#define GL_MAJOR 3
#define GL_MINOR 3

using ntf::shogle::keycode;
using ntf::shogle::scancode;
using ntf::shogle::keystate;
using ntf::shogle::keymod;

static struct {
  GLFWwindow* handle;
  std::function<void(size_t,size_t)> viewport_event;
  std::function<void(keycode,scancode,keystate,keymod)> key_event;
  std::function<void(double,double)> cursor_event;
  std::function<void(double,double)> scroll_event;
} window_state;

namespace ntf::shogle {

void engine_init(size_t width, size_t height, std::string_view title) {
  glfwInit();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHintString(GLFW_X11_CLASS_NAME, title.data());

  // Create window
  if (!(window_state.handle = glfwCreateWindow(width, height, title.data(), NULL, NULL))) {
    glfwTerminate();
  }
  glfwMakeContextCurrent(window_state.handle);

  // Event callbacks
  glfwSetFramebufferSizeCallback(window_state.handle, [](auto, int w, int h) {
    if (window_state.viewport_event) {
      window_state.viewport_event(static_cast<size_t>(w), static_cast<size_t>(h));
    }
  });
  glfwSetKeyCallback(window_state.handle, [](auto, int code, int scan, int state, int mod) {
    if (window_state.key_event) {
      window_state.key_event(
        static_cast<keycode>(code),
        static_cast<scancode>(scan),
        static_cast<keystate>(state),
        static_cast<keymod>(mod)
      );
    }
  });
  glfwSetCursorPosCallback(window_state.handle, [](auto, double xpos, double ypos) {
    if (window_state.cursor_event) {
      window_state.cursor_event(xpos, ypos);
    }
  });
  glfwSetScrollCallback(window_state.handle, [](auto, double xoff, double yoff) {
    if (window_state.scroll_event) {
      window_state.scroll_event(xoff, yoff);
    }
  });

  log::verbose("[shogle::engine] Window created (w: {}, h: {}, t: {})", width, height, title);

  // Render things
  if (!__render_init((GLADloadproc)glfwGetProcAddress)) {
    glfwDestroyWindow(window_state.handle);
    glfwTerminate();
  }
  render_viewport(width, height);
  log::verbose("[shogle::engine] Render initialized");


  // Imgui things
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void) io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window_state.handle, true);
  ImGui_ImplOpenGL3_Init("#version 130");
  log::verbose("[shogle::engine] ImGui initiaized");

  log::debug("[shogle::engine] Initiaized");
}

void engine_destroy() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  log::verbose("[shogle::engine] ImGui destroyed");

  __render_destroy();
  log::verbose("[shogle::engine] Render destroyed");

  glfwDestroyWindow(window_state.handle);
  log::verbose("[shogle::engine] Window destroyed");

  glfwTerminate();

  log::debug("[shogle::engine] Destroyed");
}


void engine_viewport_event(std::function<void(size_t,size_t)> fun) {
  window_state.viewport_event = std::move(fun);
}

void engine_key_event(std::function<void(keycode,scancode,keystate,keymod)> fun) {
  window_state.key_event = std::move(fun);
}

void engine_cursor_event(std::function<void(double,double)> fun) {
  window_state.cursor_event = std::move(fun);
}

void engine_scroll_event(std::function<void(double,double)> fun) {
  window_state.scroll_event = std::move(fun);
}

bool engine_poll_key(keycode key) {
  return glfwGetKey(window_state.handle, key) == keystate::press;
}

ivec2 engine_window_size() {
  int w, h;
  glfwGetWindowSize(window_state.handle, &w, &h);
  return {w, h};
}

void engine_use_vsync(bool flag) {
  glfwSwapInterval(flag);
}

void engine_set_title(std::string_view title) {
  glfwSetWindowTitle(window_state.handle, title.data());
}

void engine_close_window() {
  glfwSetWindowShouldClose(window_state.handle, 1);
}

GLFWwindow* __engine_window_handle() {
  return window_state.handle;
}

} // namespace ntf::shogle
