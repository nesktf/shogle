#pragma once

#include "render/window.hpp"

#include "singleton.hpp"
#include "event.hpp"

#include <functional>

namespace ntf::shogle {

enum InputBut {
  KEY_W = GLFW_KEY_W,
  KEY_A = GLFW_KEY_A, 
  KEY_S = GLFW_KEY_S,
  KEY_D = GLFW_KEY_D,
  KEY_J = GLFW_KEY_J,
  KEY_K = GLFW_KEY_K,
  KEY_L = GLFW_KEY_L
};

enum InputAction {
  KEY_PRESS = GLFW_PRESS,
  KEY_RELEASE = GLFW_RELEASE
};

using InputListener = std::function<void()>;
using InEventId = unsigned int;

struct InputEvent {
  InputListener listener;
  InEventId id;
  InputBut but;
  InputAction action;
};

class InputHandler : public Singleton<InputHandler> {
public:
  InputHandler() = default;
  ~InputHandler() = default;

  void init(Window* win);
  void poll(void);

  InEventId register_listener(InputBut but, InputAction action, InputListener listener);
  void unregister_listener(InEventId id);
  bool is_key_pressed(InputBut but) {
    return window->is_button_pressed(static_cast<int>(but));
  }

private:
  Window* window;
  unsigned int event_count {0};
  std::vector<InputEvent> listeners;
};

}
