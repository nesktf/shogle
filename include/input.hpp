#pragma once

#include "render/window.hpp"
#include "singleton.hpp"

namespace ntf::shogle {

class InputHandler : public Singleton<InputHandler> {
public:
  InputHandler() = default;
  ~InputHandler() = default;

  void init(Window* win);
  void poll(void);

private:
  Window* window;
};

}
