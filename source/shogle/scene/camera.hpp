#pragma once

#include <shogle/core/types.hpp>

namespace ntf::shogle::scene {

struct cam_common {
  mat4 proj{1.0f}, view{1.0f};
  vec2 viewport{800.0f, 600.0f};
  float znear{0.1f}, zfar{100.0f};
};

class camera2d {
public:
  camera2d() { _ccom.zfar = 1.0f, _ccom.znear = -10.0f; update_transform(); }

public:
  void update_transform();
  
public:
  camera2d& set_viewport(vec2 viewport) {
    _ccom.viewport = viewport;
    _origin = viewport*0.5f;
    return *this;
  }

  camera2d& set_center(vec2 center) {
    _center = center;
    return *this;
  }

  camera2d& set_zoom(float zoom) {
    _zoom = vec2{zoom};
    return *this;
  }

  camera2d& set_rotation(float rot) {
    _rot = rot;
    return *this;
  }

public:
  mat4 proj() const { return _ccom.proj; }
  mat4 view() const { return _ccom.view; }

  vec2 center() const { return _center; }
  float zoom() const { return _zoom.x; }
  float rotation() const { return _rot; }

private:
  cam_common _ccom{};
  vec2 _origin{400.0f, 300.0f};
  vec2 _center{0.0f};
  vec2 _zoom{1.0f};
  float _rot{0.0f};
};

class camera3d {
public:
  camera3d() { update_transform(); }

public:
  void update_transform();

public:
  camera3d& set_viewport(vec2 viewport) {
    _ccom.viewport = viewport;
    return *this;
  }

  camera3d& set_pos(vec3 pos) {
    _pos = pos;
    return *this;
  }

  camera3d& set_dir(vec3 dir) {
    _dir = dir;
    return *this;
  }

  camera3d& set_zfar(float zfar) {
    _ccom.zfar = zfar;
    return *this;
  }
  
  camera3d& set_fov(float fov) {
    _fov = fov;
    return *this;
  }

  camera3d& toggle_ortho(bool flag) {
    _use_ortho = flag;
    return *this;
  }

public:
  mat4 proj() const { return _ccom.proj; }
  mat4 view() const { return _ccom.view; }

  vec3 pos() const { return _pos; }
  vec3 dir() const { return _dir; }
  bool ortho_flag() const { return _use_ortho; }
  float fov() const { return _fov; }
  float zfar() const { return _ccom.zfar; }

private:
  cam_common _ccom{};
  bool _use_ortho{false};
  vec3 _pos{0.0f};
  vec3 _dir{0.0f, 0.0f, -1.0f};
  vec3 _up{0.0f, 1.0f, 0.0f};
  float _fov{M_PIf*0.25f};
};

} // namespace ntf::shogle::scene
