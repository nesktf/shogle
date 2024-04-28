#include <shogle/render/backends/glfw.hpp>
#include <shogle/render/backends/gl.hpp>

#include <shogle/core/log.hpp>
#include <shogle/core/error.hpp>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#define GL_MAJOR 3
#define GL_MINOR 3

namespace ntf::render {

window::window(size_t w, size_t h, const char* w_title) :
  _win_w(w),
  _win_h(h) {

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHintString(GLFW_X11_CLASS_NAME, "shogle");

  if ((_glfw_win = glfwCreateWindow(_win_w, _win_h, w_title, NULL, NULL)) == nullptr) {
    glfwTerminate();
    throw error("[window] Failed to create GLFW window");
  }
  glfwMakeContextCurrent(_glfw_win); 
  glfwSwapInterval(1); //Vsync
  Log::verbose("[window] GLFW initialized");

  try {
    gl::instance().init((GLADloadproc)glfwGetProcAddress);
  } catch (const ntf::error& err) {
    glfwDestroyWindow(_glfw_win);
    glfwTerminate();
    throw;
  }
  gl::set_viewport(_win_w, _win_h);
  Log::verbose("[window] OpenGL initialized - ver {}.{}", GL_MAJOR, GL_MINOR);


  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(_glfw_win, true);
  ImGui_ImplOpenGL3_Init("#version 130");
  Log::verbose("[window] ImGui initialized");

};

window::~window() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(_glfw_win);
  glfwTerminate();
  Log::debug("[window] Deleted");
}

void window::close(void) {
  glfwSetWindowShouldClose(_glfw_win, true);
  Log::verbose("[window] Set window close");
}

void window::swap_buffers(void) {
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glfwSwapBuffers(_glfw_win);
}

void window::set_fb_callback(GLFWframebuffersizefun cb) {
  glfwSetFramebufferSizeCallback(_glfw_win, cb);
  Log::verbose("[window] Framebuffer callback set");
}

void window::set_key_callback(GLFWkeyfun cb) {
  glfwSetKeyCallback(_glfw_win, cb);
  Log::verbose("[window] Key callback set");
}

void window::set_cursorpos_callback(cursorposcallback_t cb) {
  glfwSetCursorPosCallback(_glfw_win, cb);
  Log::verbose("[window] Cursor pos callback set");
}

void window::set_title(const char* w_title) {
  glfwSetWindowTitle(_glfw_win, w_title);
  Log::verbose("[window] Title set: {}", w_title);
}

} // namespace ntf
