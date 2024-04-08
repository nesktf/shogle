#include "render/glwindow.hpp"

#include "log.hpp"
#include "types.hpp"

#define GL_MAJOR 3
#define GL_MINOR 3

namespace ntf {

GLWindow::GLWindow(size_t w, size_t h, const char* w_title) :
  w_width(w),
  w_height(h) {

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  if ((this->win = glfwCreateWindow(w_width, w_height, w_title, NULL, NULL)) == nullptr) {
    glfwTerminate();
    throw shogle_error("[GLWindow] Failed to create GLFW window");
  }
  glfwMakeContextCurrent(this->win); 
  glfwSwapInterval(1); //Vsync
  Log::verbose("[GLWindow] GLFW initialized");

  // Load opengl function pointers to glfwGetProcAddress
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    glfwDestroyWindow(this->win); 
    glfwTerminate();
    throw shogle_error("[GLWindow] Failed to load GLAD");
  }
  Log::verbose("[GLWindow] GLAD loaded");

  // Viewport things
  glViewport(0, 0, w_width, w_height); // 1,2 -> Location in window. 3,4 -> Size
  // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // lock mouse
  Log::debug("[GLWindow] Initialized - OpenGL {}.{}", GL_MAJOR, GL_MINOR);
};

GLWindow::~GLWindow() {
  glfwDestroyWindow(win);
  glfwTerminate();
  Log::debug("[GLWindow] Deleted");
}

void GLWindow::close(void) {
  glfwSetWindowShouldClose(win, true);
  Log::verbose("[GLWindow] Set window close");
}

void GLWindow::swap_buffers(void) {
  glfwSwapBuffers(win);
}

void GLWindow::set_fb_callback(GLFWframebuffersizefun cb) {
  glfwSetFramebufferSizeCallback(win, cb);
  Log::verbose("[GLWindow] Framebuffer callback set");
}

void GLWindow::set_key_callback(GLFWkeyfun cb) {
  glfwSetKeyCallback(win, cb);
  Log::verbose("[GLWindow] Key callback set");
}

void GLWindow::set_title(const char* w_title) {
  glfwSetWindowTitle(win, w_title);
  Log::verbose("[GLWindow] Title set: {}", w_title);
}

} // namespace ntf
