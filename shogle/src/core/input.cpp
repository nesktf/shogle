#include <shogle/core/input.hpp>

#include <algorithm>

namespace ntf {

input_event::input_event(glfw::window win) :
  _win(win) {}

id_t input_event::subscribe(glfw::key key, glfw::key_action key_action, event_t event) {
  _key_events.emplace_back(
    ++_event_count,
    std::move(event),
    key, key_action
  );
  return _event_count;
}

bool input_event::unsuscribe(id_t id) {
  auto match = [id](auto& event) { return event.id == id; };
  auto it = std::find_if(_key_events.begin(), _key_events.end(), match);
  if (it != _key_events.end()) {
    _key_events.erase(it);
    return true;
  }
  return false;
}

void input_event::fire(glfw::key key, glfw::key_action key_action) {
  for (auto& action : _key_events) {
    if (action.key == key && action.key_action == key_action) {
      action.fun();
    }
  }
}

void input_event::clear(void) {
  _key_events.clear();
}

} // namespace ntf
