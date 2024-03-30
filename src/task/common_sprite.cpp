#include "task/common_sprite.hpp"
#include "render/sprite.hpp"
#include "log.hpp"

namespace ntf::shogle::task {

SpriteTask spr_init_transform(glm::vec2 pos, glm::vec2 scale, float rot) { 
  return [pos, scale, rot](auto* obj, float, float) -> bool {
    obj->update_model(TransformData{
      .pos = glm::vec3{pos, 0.0f},
      .scale = glm::vec3{scale, 1.0f},
      .rot = glm::vec3{0.0f, 0.0, rot}
    });
    return true;
  };
}

SpriteTask spr_rot_right(float ang_speed, float time) {
  return [ang_speed, time](auto* obj, float dt, float t) -> bool {
    if (t > time || time < 0.0f) {
      return true;
    }
    TransformData transform = obj->transform;
    transform.rot.z += ang_speed*dt;
    obj->update_model(transform);
    return false;
  };
}

SpriteTask spr_rot_left(float ang_speed, float time) {
  return [ang_speed, time](auto* obj, float dt, float t) -> bool {
    if (t > time || time < 0.0f) {
      return true;
    }
    TransformData transform = obj->transform;
    transform.rot.z -= ang_speed*dt;
    obj->update_model(transform);
    return false;
  };
}

} //namespace ntf::shogle::task
