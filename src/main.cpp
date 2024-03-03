#include <iostream>
#include "core/glfw.hpp"
#include "core/logger.hpp"


int main(int argc, char* argv[]) {
  using namespace ntf::shogle;
  logger::set_level(logger::LogLevel::LOG_DEBUG);

  GLFWwindow* win;
  if ((win=glfw_init(800,600,"shogle")) == nullptr) {
    logger::fatal(":(");
  }

  while (!glfwWindowShouldClose(win)) {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(win);
    glfwPollEvents();
  }

  glfw_destroy(win);
}

