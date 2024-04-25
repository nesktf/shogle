#pragma once

#include <shogle/core/types.hpp>
#include <shogle/core/window.hpp>
#include <shogle/core/singleton.hpp>

#include <functional>

namespace ntf::input {

using InputListener = std::function<void()>;
using InEventId = unsigned int;

struct InputEvent {
  InputListener listener;
  InEventId id;
  InputButtons but;
  InputAction action;
};

class InputHandler : public Singleton<InputHandler> {
public:
  InputHandler() = default;
  ~InputHandler() = default;

  void init(GLWindow* win);
  void poll(void);

  InEventId register_listener(InputButtons but, InputAction action, InputListener listener);
  void unregister_listener(InEventId id);
  void unregister_all(void);
  bool is_key_pressed(InputButtons but) {
    return _window->is_button_pressed(static_cast<int>(but));
  }

private:
  GLWindow* _window;
  uint _event_count{0};
  std::vector<InputEvent> _listeners;
};

} // namespace ntf::input
