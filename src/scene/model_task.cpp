#include "scene/model_task.hpp"

namespace ntf {

// mod_fumo_jump::mod_fumo_jump(ModelObj* _obj, float _ang_speed, float _jump_speed, float _time) :
//   ObjTask<ModelObj>(_obj),
//   ang_speed(_ang_speed),
//   jump_speed(_jump_speed),
//   time(_time),
//   half_scale(_obj->get_transform().scale.y*0.5f) {}
// void mod_fumo_jump::task(float dt) {
//   auto transform = obj->get_transform();
//
//   transform.scale.y = half_scale+(half_scale*glm::abs(glm::sin(jump_speed*t)));
//   transform.rot.y += ang_speed*dt;
//   obj->set_transform(transform);
//
//   t += dt;
//   set_task_finished(time < 0.0f ? false : t >= time);
// }
//
// mod_sin_jump::mod_sin_jump(ModelObj* _obj, float _force, float _speed, float _time) :
//   ObjTask<ModelObj>(_obj),
//   force(_force),
//   time(_time),
//   speed(_speed),
//   base_y(_obj->get_transform().pos.y) {}
// void mod_sin_jump::task(float dt) {
//   auto transform = obj->get_transform();
//
//   transform.pos.y = base_y + (force*glm::abs(glm::sin(speed*t)));
//   obj->set_transform(transform);
//
//   t += dt;
//   set_task_finished(time < 0.0f ? false : t >= time);
// }

// ModelTask mod_linear_rel_move(glm::vec3 new_pos, float time) {
//   return [new_pos, time](auto* obj, float dt, float t) -> bool {
//     TransformData transform = obj->transform;
//
//     static glm::vec3 vel = new_pos/time;
//     transform.pos += vel*dt;
//     obj->update_model(transform);
//
//     return (t >= time);
//   };
// }
//
// ModelTask mod_linear_abs_move(glm::vec3 new_pos, float time) {
//   return [new_pos, time](auto* obj, float dt, float t) -> bool {
//     TransformData transform = obj->transform;
//
//     static glm::vec3 vel = (new_pos - transform.pos)/time;
//     transform.pos += vel*dt;
//     obj->update_model(transform);
//
//     return (t >= time);
//   };
// }
//
// ModelTask mod_funny_jump(float force, float time) {
//   return [force, time](auto* obj, float, float t) -> bool {
//     TransformData transform = obj->transform;
//
//     static float base_y = transform.pos.y;
//     transform.pos.y = base_y + force*glm::abs(glm::sin(t));
//     obj->update_model(transform);
//
//     return (t >= time);
//   };
// }

} // namespace ntf
