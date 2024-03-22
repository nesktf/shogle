#include "render/window.hpp"

#define GL_MAJOR 3
#define GL_MINOR 3

#include <exception>

namespace ntf::shogle {

Window::Window(size_t w, size_t h, const char* w_title) :
  w_width(w),
  w_height(h) {

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  if ((this->win = glfwCreateWindow(w_width, w_height, w_title, NULL, NULL)) == nullptr) {
    glfwTerminate();
    throw std::runtime_error("[Window] Failed to create GLFW window");
  }
  glfwMakeContextCurrent(this->win); 
  glfwSwapInterval(1); //Vsync
  Log::verbose("[Window] GLFW initialized");


  // Load opengl function pointers to glfwGetProcAddress
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    glfwDestroyWindow(this->win); 
    glfwTerminate();
    throw std::runtime_error("[Window] Failed to load GLAD");
  }
  Log::verbose("[Window] GLAD loaded");

  // Viewport things
  glViewport(0, 0, w_width, w_height); // 1,2 -> Location in window. 3,4 -> Size
  // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // lock mouse
  Log::debug("[Window] Initialized - OpenGL {}.{}", GL_MAJOR, GL_MINOR);
};

Window::~Window() {
  glfwDestroyWindow(win);
  glfwTerminate();
  Log::debug("[Window] Deleted");
}

} // namespace ntf::shogle
