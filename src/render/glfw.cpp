#include "render/glfw.hpp"
#include "log.hpp"

#define GL_MAJOR 3
#define GL_MINOR 3

namespace ntf::shogle::window {

GLFWwindow* window;

bool init(size_t w_width, size_t w_height, const char* w_name) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  if ((window = glfwCreateWindow(w_width, w_height, w_name, NULL, NULL)) == nullptr) {
    Log::error("[GLFW] Failed to create window");
    glfwTerminate();
    return false;
  }
  glfwMakeContextCurrent(window); 
  glfwSwapInterval(1); //Vsync
  Log::verbose("[GLFW] Window initialized");

  // Load opengl function pointers to glfwGetProcAddress
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    Log::error("[GLFW] Failed to initalize GLAD");
    glfwDestroyWindow(window); glfwTerminate();
    return false;
  }
  Log::verbose("[GLFW] GLAD loaded");

  // Viewport things
  glViewport(0, 0, w_width, w_height); // 1,2 -> Location in window. 3,4 -> Size
  // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // lock mouse
  Log::debug("[GLFW] Initialized - OpenGL {}.{}", GL_MAJOR, GL_MINOR);
  return true;
}

void destroy(void) {
  glfwDestroyWindow(window);
  Log::verbose("[GLFW] Window destroyed");
  glfwTerminate();
  Log::debug("[GLFW] Terminated");
}

void set_close(void) {
  glfwSetWindowShouldClose(window, true);
  Log::verbose("[GLFW] Window set should close");
}

void swap_buffer(void) {
  glfwSwapBuffers(window);
}

bool should_close(void) {
  return glfwWindowShouldClose(window);
}

void set_fb_callback(GLFWframebuffersizefun callback) {
  glfwSetFramebufferSizeCallback(window, callback);
  Log::verbose("[GLFW] FramebufferSize Callback modified");
}

}
