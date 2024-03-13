#pragma once

#include "core/manipulator.hpp"

namespace ntf::shogle {

class SinJump3D : public Manipulator {
public:
  SinJump3D(GameObject& obj, float ang_speed = 200.0f, float jump_amp = 9.0f) :
    Manipulator(obj),
    ang_speed(ang_speed),
    jump_amp(jump_amp) {}

public:
  void update(float delta_time) override;

protected:
  float ang_speed, jump_amp;
};

} // namespace ntf::shogle
