#pragma once

#include "core/window.hpp"
#include "core/singleton.hpp"

#include <functional>

namespace ntf {

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
  bool is_key_pressed(InputButtons but) {
    return window->is_button_pressed(static_cast<int>(but));
  }

private:
  GLWindow* window;
  unsigned int event_count {0};
  std::vector<InputEvent> listeners;
};

}
