#include "task/common_sprite.hpp"

#include "render/sprite.hpp"
#include "level/game_object.hpp"

#include "log.hpp"

static inline glm::vec3 sprite_pos(glm::vec2 pos) {
  return glm::vec3{pos, 0.0f};
}
static inline glm::vec3 sprite_scale(glm::vec2 scale) {
  return glm::vec3{scale, 1.0f};
}
static inline glm::vec3 sprite_rot(float rot) {
  return glm::vec3{0.0f, 0.0f, rot};
}

namespace ntf::shogle::task {

spr_transform::spr_transform(SpriteObj* _obj, glm::vec2 pos, glm::vec2 scale, float rot) :
    Task<SpriteObj>(_obj),
    transform(TransformData{
      .pos = sprite_pos(pos),
      .scale = sprite_scale(scale),
      .rot = sprite_rot(rot)
    }) {}
bool spr_transform::operator()(float) {
  obj->set_transform(transform);
  return true;
}

spr_rotate::spr_rotate(SpriteObj* _obj, float _ang_speed, float _time) :
  Task<SpriteObj>(_obj),
  ang_speed(_ang_speed),
  time(_time) {}
bool spr_rotate::operator()(float dt) {
  TransformData transform = obj->get_transform();

  transform.rot.z += ang_speed*dt;
  obj->set_transform(transform);

  t += dt;
  return (t >= time);
}

spr_move_circle::spr_move_circle(SpriteObj* _obj, glm::vec2 _center, float _ang_speed, float _radius, float _time) :
  Task<SpriteObj>(_obj),
  ang_speed(_ang_speed),
  radius(_radius),
  time(_time),
  center(_center) {}
bool spr_move_circle::operator()(float dt) {
  TransformData transform = obj->get_transform();

  transform.pos = sprite_pos(glm::vec2{
    center.x+radius*glm::cos(ang_speed*t),
    center.y+radius*glm::sin(ang_speed*t)
  });
  obj->set_transform(transform);

  t += dt;
  return (t >= time);
}

} //namespace ntf::shogle::task
