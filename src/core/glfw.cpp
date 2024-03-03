#include "core/glfw.hpp"
#include "core/logger.hpp"

#define GL_MAJOR 3
#define GL_MINOR 3

namespace ntf::shogle {

GLFWwindow* glfw_init(unsigned int w_width, unsigned int w_height, const char* w_name) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* win;
  if ((win = glfwCreateWindow(w_width, w_height, w_name, NULL, NULL)) == nullptr) {
    logger::error("[GLFW] Failed to create window");
    glfwTerminate();
    return nullptr;
  }
  glfwMakeContextCurrent(win); 
  glfwSwapInterval(1); //Vsync

  // Load opengl function pointers to glfwGetProcAddress
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    logger::error("[GLFW] Failed to initalize GLAD");
    glfwDestroyWindow(win);
    glfwTerminate();
    return nullptr;
  }

  // Viewport things
  glViewport(0, 0, w_width, w_height); // 1,2 -> Location in window. 3,4 -> Size
  glfwSetFramebufferSizeCallback(win, [](GLFWwindow* win, int w, int h) {
    glViewport(0, 0, w, h);
  });

  logger::debug("[GLFW] Initialized - OpenGL {}.{}", GL_MAJOR, GL_MINOR);
  return win;
}

void glfw_destroy(GLFWwindow* win) {
  glfwDestroyWindow(win);
  glfwTerminate();
  logger::debug("[GLFW] Terminated");
}

}
