#include <shogle/input/input.hpp>

#include <shogle/core/log.hpp>

namespace ntf::input {

void KeyListener::init(void) {
  backend::instance().set_key_callback([](auto, int key, int, int action, int) {
    auto& _this = KeyListener::instance();
    for (auto& event : _this._listeners) {
      if (key == static_cast<int>(event.key) && action == static_cast<int>(event.action)) {
        event.listener();
      }
    }
  });
}

KeyListener::listenerid_t KeyListener::register_listener(keys key, key_action action, listenerfun_t listener) {
  _listeners.emplace_back(key_listener{
    .listener = listener,
    .id = static_cast<listenerid_t>(++_event_count), 
    .key = key, 
    .action = action
  });
  return _event_count;
}

bool KeyListener::unregister_listener(listenerid_t listener) {
  auto match_id = [listener](auto& event) { return event.id == listener; };
  if (auto it = std::find_if(_listeners.begin(), _listeners.end(), match_id); it!=_listeners.end()) {
    _listeners.erase(it);
    return true;
  }
  return false;
}

} // namespace ntf::input
