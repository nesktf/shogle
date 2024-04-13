#include "scene/camera3d.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace ntf {

Camera3D::Camera3D(proj_info p_info) :
  _proj_info(p_info) { 
    _proj = _gen_proj(_proj_info); 
    _view = _gen_view(_view_info);
}

Camera3D::Camera3D(proj_info p_info, view_info v_info) :
  _proj_info(p_info),
  _view_info(v_info) { 
    _proj = _gen_proj(_proj_info);
    _view = _gen_view(_view_info);
}

void Camera3D::upd_target(vec2 screen_pos, float sensitivity) {
  if (_first_movement) {
    _last_screen_pos = screen_pos;
    _first_movement = false;
  }
  float dx = (screen_pos.x - _last_screen_pos.x) * sensitivity;
  float dy = (_last_screen_pos.y - screen_pos.y) * sensitivity;
  _last_screen_pos = screen_pos;
  
  set_dir(cam_dir{
    .yaw = glm::radians(dx),
    .pitch = glm::clamp(glm::radians(dy), -M_PIf*0.5f, M_PIf*0.5f)
  });
}

vec3 Camera3D::dir_to_vec(cam_dir dir) {
  vec3 dir_;

  dir_.x = glm::cos(dir.yaw) * glm::cos(dir.pitch);
  dir_.y = glm::sin(dir.pitch);
  dir_.z = glm::sin(dir.yaw) * glm::cos(dir.pitch);

  vec3 dir_vec = glm::normalize(dir_);

  return dir_vec;
}

Camera3D::cam_dir Camera3D::vec_to_dir(vec3 dir_vec) {
  cam_dir dir{};

  dir.pitch = glm::asin(-dir_vec.y);
  dir.yaw = std::atan2(dir_vec.x, dir_vec.z);

  return dir;
}

mat4 Camera3D::_gen_proj(proj_info p_info) {
  return glm::perspective(
    p_info.fov,
    p_info.viewport.x/p_info.viewport.y,
    p_info.draw_dist.x, p_info.draw_dist.y
  );
}

mat4 Camera3D::_gen_view(view_info v_info) {
  return glm::lookAt(
    v_info.pos,
    v_info.pos + v_info.dir_vec,
    v_info.up
  );
}

} // namespace ntf
