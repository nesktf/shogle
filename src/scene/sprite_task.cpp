#include "scene/sprite_task.hpp"

namespace ntf {

// spr_rotate::spr_rotate(SpriteObj* _obj, float _ang_speed, float _time) :
//   ObjTask<SpriteObj>(_obj),
//   ang_speed(_ang_speed),
//   time(_time) {}
// void spr_rotate::task(float dt) {
//   TransformData transform = obj->get_transform();
//
//   transform.rot.z += ang_speed*dt;
//   obj->set_transform(transform);
//
//   t += dt;
//   
//   set_task_finished(time < 0.0f ? false : t >= time);
// }
//
// spr_move_circle::spr_move_circle(SpriteObj* _obj, glm::vec2 _center, float _ang_speed, float _radius, float _time) :
//   ObjTask<SpriteObj>(_obj),
//   ang_speed(_ang_speed),
//   radius(_radius),
//   time(_time),
//   center(_center) {}
// void spr_move_circle::task(float dt) {
//   TransformData transform = obj->get_transform();
//
//   transform.pos = math::sprite_pos(glm::vec2{
//     center.x+radius*glm::cos(ang_speed*t),
//     center.y+radius*glm::sin(ang_speed*t)
//   });
//   obj->set_transform(transform);
//
//   t += dt;
//   set_task_finished(time < 0.0f ? false : t >= time);
// }

} //namespace ntf
