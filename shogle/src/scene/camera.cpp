#include <shogle/scene/camera.hpp>

namespace ntf {

void camera2D::update(float) {
  _proj = glm::ortho(
    0.0f, _viewport.x,
    _viewport.y, 0.0f,
    -static_cast<float>(_layer_count), 1.0f
  );

  mat4 view {1.0f};

  view = glm::translate(view, vec3{_origin, 0.0f});
  view = glm::rotate(view, _rot, vec3{0.0f, 0.0f, 1.0f});
  view = glm::scale(view, vec3{_zoom, 1.0f});
  view = glm::translate(view, vec3{-_center, 0.0f});

  _view = view;
}


void camera3D::update(float) {
  if (_use_ortho) {
    _proj = glm::ortho(
      0.0f, _viewport.x,
      _viewport.y, 0.0f,
      _draw_dist.x, _draw_dist.y
    );
  } else {
    _proj = glm::perspective(
      _fov,
      _viewport.x/_viewport.y,
      _draw_dist.x, _draw_dist.y
    );
  }
  _view = glm::lookAt(
    _pos,
    _pos + _dir,
    _up
  );
}


// void camera3D::upd_target(vec2 screen_pos, float sensitivity) {
//   if (_first_movement) {
//     _last_screen_pos = screen_pos;
//     _first_movement = false;
//   }
//   float dx = (screen_pos.x - _last_screen_pos.x) * sensitivity;
//   float dy = (_last_screen_pos.y - screen_pos.y) * sensitivity;
//   _last_screen_pos = screen_pos;
//   
//   set_dir(cam_dir{
//     .yaw = glm::radians(dx),
//     .pitch = glm::clamp(glm::radians(dy), -M_PIf*0.5f, M_PIf*0.5f)
//   });
// }
//
// vec3 camera3D::dir_to_vec(cam_dir dir) {
//   vec3 dir_;
//
//   dir_.x = glm::cos(dir.yaw) * glm::cos(dir.pitch);
//   dir_.y = glm::sin(dir.pitch);
//   dir_.z = glm::sin(dir.yaw) * glm::cos(dir.pitch);
//
//   vec3 dir_vec = glm::normalize(dir_); // Necesary?
//
//   return dir_vec;
// }
//
// camera3D::cam_dir camera3D::vec_to_dir(vec3 dir_vec, vec3) {
//   cam_dir dir{};
//
//   dir.pitch = glm::asin(-dir_vec.y);
//   dir.yaw = std::atan2(dir_vec.x, dir_vec.z);
//
//   return dir;
// }

}; // namespace ntf::scene
