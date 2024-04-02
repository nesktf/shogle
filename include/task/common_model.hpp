#pragma once

#include "task/task.hpp"

#include "render/model.hpp"

#include "level/game_object.hpp"

namespace ntf::shogle::task {

struct mod_fumo_jump : public ObjTask<ModelObj> {
  mod_fumo_jump(ModelObj* _obj, float _ang_speed, float _jump_speed, float _time);
  void task(float dt) override;

  float ang_speed, jump_speed, time, half_scale;
  float t {0.0f};
};

struct mod_sin_jump : public ObjTask<ModelObj> {
  mod_sin_jump(ModelObj* _obj, float _force, float _speed, float _time);
  void task(float dt) override;

  float force, time, speed, base_y;
  float t {0.0f};
};

} // namsepace ntf::shogle
