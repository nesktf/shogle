#pragma once

#include "task/task.hpp"

namespace ntf::shogle {
template<typename TObj>
class GameObject;
}

namespace ntf::shogle::render {
class Sprite;
}

namespace ntf::shogle::task {

using SpriteObj = GameObject<render::Sprite>;

struct spr_transform : public Task<SpriteObj> {
  spr_transform(SpriteObj* _obj, glm::vec2 pos, glm::vec2 scale, float rot);
  bool operator()(float dt) override;

  TransformData transform;
};

struct spr_rotate : public Task<SpriteObj> {
  spr_rotate(SpriteObj* _obj, float _ang_speed, float _time);
  bool operator()(float dt) override;

  float ang_speed, time;
  float t {0.0f};
};

struct spr_move_circle : public Task<SpriteObj> {
  spr_move_circle(SpriteObj* _obj, glm::vec2 _center, float _ang_speed, float _radius, float _time);
  bool operator()(float dt) override;

  float ang_speed, radius, time;
  glm::vec2 center;
  float t {0.0f};
};

} //namespace ntf::shogle::task
