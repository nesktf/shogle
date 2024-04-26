#include <shogle/scene/camera.hpp>

namespace ntf {

Camera2D::Camera2D(data_t::proj proj) :
  _proj(proj) { update({}); }
Camera2D::Camera2D(data_t::proj proj, data_t::view view):
  _proj(proj),
  _view(view) { update({}); }

void Camera2D::update(float) {
  if (_proj.upd) {
    _proj.mat = glm::ortho(
      0.0f, _proj.viewport.x,
      _proj.viewport.y, 0.0f,
      -static_cast<float>(_proj.layer_count), 1.0f
    );
    _proj.upd = false;
  }

  if (_view.upd) {
    mat4 view {1.0f};

    view = glm::translate(view, vec3{_view.origin, 0.0f});
    view = glm::rotate(view, _view.rot, vec3{0.0f, 0.0f, 1.0f});
    view = glm::scale(view, vec3{_view.zoom, 1.0f});
    view = glm::translate(view, vec3{-_view.center, 0.0f});

    _view.mat = view;
    _view.upd = false;
  }
}

Camera3D::Camera3D(data_t::proj proj) :
  _proj(proj) { update({}); }
Camera3D::Camera3D(data_t::proj proj, data_t::view view) :
  _proj(proj),
  _view(view) { update({}); }

void Camera3D::update(float) {
  if (_proj.upd) {
    if (_proj.use_ortho) {
      _proj.mat = glm::ortho(
        0.0f, _proj.viewport.x,
        _proj.viewport.y, 0.0f,
        _proj.draw_dist.x, _proj.draw_dist.y
      );
    } else {
      _proj.mat = glm::perspective(
        _proj.fov,
        _proj.viewport.x/_proj.viewport.y,
        _proj.draw_dist.x, _proj.draw_dist.y
      );
    }
    _proj.upd = false;
  }

  if (_view.upd) {
    _view.mat = glm::lookAt(
      _view.pos,
      _view.pos + _view.dir,
      _view.up
    );
    _view.upd = false;
  }
}

// void Camera3D::upd_target(vec2 screen_pos, float sensitivity) {
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
// vec3 Camera3D::dir_to_vec(cam_dir dir) {
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
// Camera3D::cam_dir Camera3D::vec_to_dir(vec3 dir_vec, vec3) {
//   cam_dir dir{};
//
//   dir.pitch = glm::asin(-dir_vec.y);
//   dir.yaw = std::atan2(dir_vec.x, dir_vec.z);
//
//   return dir;
// }

}; // namespace ntf
