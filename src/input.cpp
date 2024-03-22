#include "input.hpp"
#include "log.hpp"

namespace ntf::shogle {

void InputHandler::init(Window* win) {
  this->window = win;
  Log::debug("[InputHandler] Initialized");
}

void InputHandler::poll(void) {
  window->poll_events();
}

}
