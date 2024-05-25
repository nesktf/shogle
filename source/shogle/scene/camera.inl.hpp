#define CAMERA_INL_HPP
#include <shogle/scene/camera.hpp>
#undef CAMERA_INL_HPP

namespace ntf::shogle::scene {

inline camera2d& camera2d::set_viewport(vec2 viewport) {
  _ccom.viewport = viewport;
  _origin = viewport*0.5f;
  return *this;
}

inline camera2d& camera2d::set_center(vec2 center) {
  _center = center;
  return *this;
}

inline camera2d& camera2d::set_zoom(float zoom) {
  _zoom = vec2{zoom};
  return *this;
}

inline camera2d& camera2d::set_rotation(float rot) {
  _rot = rot;
  return *this;
}

inline camera3d& camera3d::set_viewport(vec2 viewport) {
  _ccom.viewport = viewport;
  return *this;
}

inline camera3d& camera3d::set_pos(vec3 pos) {
  _pos = pos;
  return *this;
}

inline camera3d& camera3d::set_dir(vec3 dir) {
  _dir = dir;
  return *this;
}

inline camera3d& camera3d::set_zfar(float zfar) {
  _ccom.zfar = zfar;
  return *this;
}

inline camera3d& camera3d::set_fov(float fov) {
  _fov = fov;
  return *this;
}

inline camera3d& camera3d::toggle_ortho(bool flag) {
  _use_ortho = flag;
  return *this;
}

}
