#pragma once

#include "singleton.hpp"

namespace ntf::shogle {

class InputHandler : public Singleton<InputHandler> {
public:
  InputHandler() = default;
  ~InputHandler() = default;

  void init(void);
  void poll(void);
};

}
