#pragma once

#include <shogle/core/types.hpp>
#include <shogle/scene/object.hpp>

namespace ntf::shogle::scene {

class cam_base {
protected:
  cam_base() = default;

public:
  mat4 proj() { return _proj; }
  mat4 view() { return _view; }

protected:
  mat4 _proj{1.0f}, _view{1.0f};

  vec2 _viewport{800.0f, 600.0f};
  float _znear{0.1f};
  float _zfar{100.0f};
};

class camera2d : public cam_base {
public:
  camera2d() { _zfar = 1.0f, _znear = -10.0f; }

public:
  void update();
  
public:
  camera2d& set_viewport(vec2 viewport) {
    _viewport = viewport;
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
  vec2 center() const { return _center; }
  float zoom() const { return _zoom.x; }
  float rotation() const { return _rot; }

private:
  vec2 _origin{400.0f, 300.0f};
  vec2 _center{0.0f};
  vec2 _zoom{1.0f};
  float _rot{0.0f};
};

class camera3d : public cam_base {
public:
  camera3d() = default;

public:
  void update();

public:
  camera3d& set_viewport(vec2 viewport) {
    _viewport = viewport;
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
    _zfar = zfar;
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
  vec3 pos() const { return _pos; }
  vec3 dir() const { return _dir; }
  bool ortho_flag() const { return _use_ortho; }
  float fov() const { return _fov; }
  float zfar() const { return _zfar; }

private:
  bool _use_ortho{false};
  vec3 _pos{0.0f};
  vec3 _dir{0.0f, 0.0f, -1.0f};
  vec3 _up{0.0f, 1.0f, 0.0f};
  float _fov{M_PIf*0.25f};
};

} // namespace ntf::shogle::scene
