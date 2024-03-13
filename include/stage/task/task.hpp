#pragma once

#include "core/game_object.hpp"
#include <functional>

namespace ntf::shogle {

class Manipulator {
public:
  virtual ~Manipulator() = default;

public:
  virtual void update(float delta_time) = 0;

protected:
  Manipulator(GameObject& obj) :
    object(obj) {}

public:
  std::reference_wrapper<GameObject> object;
  bool is_complete = false;
};

} // namespace ntf::shogle
