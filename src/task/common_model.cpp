#include "task/common_model.hpp"

#include "render/model.hpp"
#include "level/game_object.hpp"

#include "log.hpp"

namespace ntf::shogle::task {

// ModelTask mod_transform(glm::vec3 pos, glm::vec3 scale, glm::vec3 rot) {
//   TransformData data {
//     .pos = pos,
//     .scale = scale,
//     .rot = rot
//   };
//   return [data](auto* obj, float, float) -> bool {
//     obj->update_model(data);
//     return true;
//   };
// }
//
// ModelTask mod_fumo_jump(float ang_speed, float jump_speed, float time) {
//   return [ang_speed, jump_speed, time](auto* obj, float dt, float t) -> bool {
//     TransformData transform = obj->transform;
//
//     static float half_scale = transform.scale.y*0.5f;
//     transform.scale.y = half_scale + (half_scale*glm::abs(glm::sin(jump_speed*t)));
//     transform.rot.y += ang_speed * dt;
//     obj->update_model(transform);
//
//     return (t >= time || time < 0.0f);
//   };
// }
//
// ModelTask mod_z_rot(float ang_speed, float time) {
//   return [ang_speed, time](auto* obj, float dt, float t) -> bool {
//     TransformData transform = obj->transform;
//     
//     transform.rot.z += ang_speed*dt;
//     obj->update_model(transform);
//
//     return (t >= time || time < 0.0f);
//   };
// }
//
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

} // namespace ntf::shogle
