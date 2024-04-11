#include "core/input.hpp"
#include "core/log.hpp"

namespace ntf {

void InputHandler::init(GLWindow* win) {
  this->window = win;
  win->set_key_callback([](auto, int key, int, int action, int) {
    auto& handler = InputHandler::instance();
    for (auto& event : handler.listeners) {
      if (key == static_cast<int>(event.but) && action == static_cast<int>(event.action)) {
        event.listener();
      }
    }
  });
  Log::debug("[InputHandler] Initialized");
}

void InputHandler::poll(void) {
  window->poll_events();
}

InEventId InputHandler::register_listener(InputButtons but, InputAction action, InputListener listener) {
  listeners.push_back(InputEvent{.listener = listener, .id = ++event_count, .but = but, .action = action});
  return event_count;
}

void InputHandler::unregister_listener(InEventId id) {
  auto match_id = [id](auto& event) { return event.id == id; };
  if (auto it = std::find_if(listeners.begin(), listeners.end(), match_id); it!=listeners.end()) {
    listeners.erase(it);
  }
}

void InputHandler::unregister_all(void) {
  listeners.clear();
}

} // namespace ntf
