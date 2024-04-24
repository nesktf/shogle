#include <shogle/core/input.hpp>
#include <shogle/core/log.hpp>

namespace ntf {

void InputHandler::init(GLWindow* win) {
  this->_window = win;
  win->set_key_callback([](auto, int key, int, int action, int) {
    auto& handler = InputHandler::instance();
    for (auto& event : handler._listeners) {
      if (key == static_cast<int>(event.but) && action == static_cast<int>(event.action)) {
        event.listener();
      }
    }
  });
  Log::debug("[InputHandler] Initialized");
}

void InputHandler::poll(void) {
  _window->poll_events();
}

InEventId InputHandler::register_listener(InputButtons but, InputAction action, InputListener listener) {
  _listeners.push_back(InputEvent{.listener = listener, .id = ++_event_count, .but = but, .action = action});
  return _event_count;
}

void InputHandler::unregister_listener(InEventId id) {
  auto match_id = [id](auto& event) { return event.id == id; };
  if (auto it = std::find_if(_listeners.begin(), _listeners.end(), match_id); it!=_listeners.end()) {
    _listeners.erase(it);
  }
}

void InputHandler::unregister_all(void) {
  _listeners.clear();
}

} // namespace ntf
