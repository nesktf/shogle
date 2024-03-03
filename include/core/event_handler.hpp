#pragma once

#include "util/singleton.hpp"

namespace ntf::shogle {

class EventHandler : public Singleton<EventHandler> {
public:
  EventHandler(){}
  ~EventHandler();

  void init(void);
  void poll(void);
};

}
