#include <shogle/core/window.hpp>
#include <shogle/core/log.hpp>
#include <shogle/core/error.hpp>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#define GL_MAJOR 3
#define GL_MINOR 3

namespace ntf {

GLWindow::GLWindow(size_t w, size_t h, const char* w_title) :
  _win_w(w),
  _win_h(h) {

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHintString(GLFW_X11_CLASS_NAME, "shogle");

  if ((this->_glfw_win = glfwCreateWindow(_win_w, _win_h, w_title, NULL, NULL)) == nullptr) {
    glfwTerminate();
    throw error("[GLWindow] Failed to create GLFW window");
  }
  glfwMakeContextCurrent(this->_glfw_win); 
  glfwSwapInterval(1); //Vsync
  Log::verbose("[GLWindow] GLFW initialized");

  // Load opengl function pointers to glfwGetProcAddress
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    glfwDestroyWindow(this->_glfw_win); 
    glfwTerminate();
    throw error("[GLWindow] Failed to load GLAD");
  }
  Log::verbose("[GLWindow] GLAD loaded");

  // Viewport things
  glViewport(0, 0, _win_w, _win_h); // 1,2 -> Location in window. 3,4 -> Size
  // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // lock mouse


  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(_glfw_win, true);
  ImGui_ImplOpenGL3_Init("#version 130");

  Log::debug("[GLWindow] Initialized - OpenGL {}.{}", GL_MAJOR, GL_MINOR);
};

GLWindow::~GLWindow() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(_glfw_win);
  glfwTerminate();
  Log::debug("[GLWindow] Deleted");
}

void GLWindow::close(void) {
  glfwSetWindowShouldClose(_glfw_win, true);
  Log::verbose("[GLWindow] Set window close");
}

void GLWindow::swap_buffers(void) {
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glfwSwapBuffers(_glfw_win);
}

void GLWindow::set_fb_callback(GLFWframebuffersizefun cb) {
  glfwSetFramebufferSizeCallback(_glfw_win, cb);
  Log::verbose("[GLWindow] Framebuffer callback set");
}

void GLWindow::set_key_callback(GLFWkeyfun cb) {
  glfwSetKeyCallback(_glfw_win, cb);
  Log::verbose("[GLWindow] Key callback set");
}

void GLWindow::set_cursorpos_callback(cursorposcallback_t cb) {
  glfwSetCursorPosCallback(_glfw_win, cb);
  Log::verbose("[GLWindow] Cursor pos callback set");
}

void GLWindow::set_title(const char* w_title) {
  glfwSetWindowTitle(_glfw_win, w_title);
  Log::verbose("[GLWindow] Title set: {}", w_title);
}

} // namespace ntf
