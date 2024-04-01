#include "task/common_sprite.hpp"
#include "math.hpp"

#include "log.hpp"

namespace ntf::shogle::task {

spr_transform::spr_transform(SpriteObj* _obj, glm::vec2 pos, glm::vec2 scale, float rot) :
    Task<SpriteObj>(_obj),
    transform(TransformData{
      .pos = math::sprite_pos(pos),
      .scale = math::sprite_scale(scale),
      .rot = math::sprite_rot(rot)
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
  return (time < 0.0f ? false : t >= time);
}

spr_move_circle::spr_move_circle(SpriteObj* _obj, glm::vec2 _center, float _ang_speed, float _radius, float _time) :
  Task<SpriteObj>(_obj),
  ang_speed(_ang_speed),
  radius(_radius),
  time(_time),
  center(_center) {}
bool spr_move_circle::operator()(float dt) {
  TransformData transform = obj->get_transform();

  transform.pos = math::sprite_pos(glm::vec2{
    center.x+radius*glm::cos(ang_speed*t),
    center.y+radius*glm::sin(ang_speed*t)
  });
  obj->set_transform(transform);

  t += dt;
  return (time < 0.0f ? false : t >= time);
}

} //namespace ntf::shogle::task
