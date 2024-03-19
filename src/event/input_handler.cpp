#include "event/input_handler.hpp"
#include "log.hpp"
#include "render/glfw.hpp"

namespace ntf::shogle {

void InputHandler::init(void) {

  window::set_fb_callback([](GLFWwindow* win, int w, int h) {
    glViewport(0, 0, w, h); // 1,2 -> Location in window. 3,4 -> Size
    // Renderer::instance().update_proj_m(w, h);
  });

  log::debug("[InputHandler] Initialized");
}

void InputHandler::poll(void) {
  glfwPollEvents();
}

}