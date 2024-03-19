#include "manipulators/sin_jump_3d.hpp"
#include <glm/trigonometric.hpp>

namespace ntf::shogle {

void SinJump3D::update(float delta_time) {
  auto& obj = this->object.get();

  static float time_c = 0.0f;
  float base_y = obj.scale_v.y*0.5f;

  time_c += delta_time;
  obj.scale_v.y = base_y + (base_y*glm::abs(glm::sin(jump_amp*time_c)));
  obj.rot_v.y += ang_speed * delta_time;
}

} // namespace ntf::shogle
