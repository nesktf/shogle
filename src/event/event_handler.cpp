#include "core/renderer.hpp"
#include "core/event_handler.hpp"
#include "core/logger.hpp"
#include "core/glfw.hpp"

namespace ntf::shogle {

void EventHandler::init(void) {

  window::set_fb_callback([](GLFWwindow* win, int w, int h) {
    glViewport(0, 0, w, h); // 1,2 -> Location in window. 3,4 -> Size
    Renderer::instance().update_proj_m(w, h);
  });

  logger::debug("[EventHandler] Initialized");
}

EventHandler::~EventHandler() {
  logger::debug("[EventHandler] Terminated");
}

void EventHandler::poll(void) {
  glfwPollEvents();
}

}
