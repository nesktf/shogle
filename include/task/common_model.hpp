#pragma once

#include "task/task.hpp"

namespace ntf::shogle {
template<typename TObj>
class GameObject;
}

namespace ntf::shogle::render {
class Model;
}

namespace ntf::shogle::task {

using ModelObj = GameObject<render::Model>;

struct mod_transform : public Task<ModelObj> {
  mod_transform(ModelObj* _obj, glm::vec3 pos, glm::vec3 scale, glm::vec3 rot);
  bool operator()(float dt) override;

  TransformData transform;
};

struct mod_fumo_jump : public Task<ModelObj> {
  mod_fumo_jump(ModelObj* _obj, float _ang_speed, float _jump_speed, float _time);
  bool operator()(float dt) override;

  float ang_speed, jump_speed, time, half_scale;
  float t {0.0f};
};

struct mod_sin_jump : public Task<ModelObj> {
  mod_sin_jump(ModelObj* _obj, float _force, float _speed, float _time);
  bool operator()(float dt) override;

  float force, time, speed, base_y;
  float t {0.0f};
};
// using ModelTask = Task<GameObject<render::Model>>;
//
// ModelTask mod_transform(glm::vec3 pos, glm::vec3 scale, glm::vec3 rot);
// ModelTask mod_fumo_jump(float ang_speed, float jump_speed, float time);
// ModelTask mod_z_rot(float ang_speed, float time);
// ModelTask mod_linear_rel_move(glm::vec3 new_pos, float time);
// ModelTask mod_linear_abs_move(glm::vec3 new_pos, float time);
// ModelTask mod_funny_jump(float force, float time);

} // namsepace ntf::shogle
