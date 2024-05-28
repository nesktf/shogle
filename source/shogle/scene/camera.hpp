#pragma once

#include <shogle/core/types.hpp>

namespace ntf::shogle::scene {

template<typename dim_t, typename T>
class camera_view;

template<typename dim_t>
class camera : public camera_view<dim_t, camera<dim_t>> {
public:
  camera(vec2 viewport);
  camera(float w, float h);

public:
  void update();

public:
  inline camera& set_viewport(vec2 viewport);
  inline camera& set_viewport(float w, float h);
  inline camera& set_znear(float znear);
  inline camera& set_zfar(float zfar);

public:
  mat4 proj() const { return _proj; }
  mat4 view() const { return _view; }
  float znear() const { return _znear; }
  float zfar() const { return _zfar; }
  vec2 vport() const { return _viewport; }

private:
  mat4 _proj {1.0f};
  mat4 _view {1.0f};
  float _znear {0.1f};
  float _zfar {100.0f};
  vec2 _viewport;
};

template<typename T>
class camera_view<vec2, T> {
protected:
  camera_view(vec2 viewport);

public:
  inline T& set_center(vec2 center);
  inline T& set_center(float x, float y);
  inline T& set_zoom(vec2 zoom);
  inline T& set_zoom(float x, float y);
  inline T& set_zoom(float zoom);
  inline T& set_rotation(float rot);

public:
  vec2 center() const { return _center; }
  vec2 zoom() const { return _zoom; }
  float rot() const { return _rot; }

protected:
  vec2 _center {0.0f};
  vec2 _zoom {1.0f};
  float _rot {0.0f};
  vec2 _origin;
};

template<typename T>
class camera_view<vec3, T> {
protected:
  camera_view(vec2 viewport);

public:
  inline T& set_position(vec3 pos);
  inline T& set_position(float x, float y, float z);
  inline T& set_direction(vec3 dir);
  inline T& set_direction(float x, float y, float z);
  inline T& set_fov(float fov);
  inline T& use_perspective();
  inline T& use_orthographic();

public:
  bool is_persp() const { return _use_persp; }
  bool is_ortho() const { return !_use_persp; }
  vec3 pos() const { return _pos; }
  vec3 dir() const { return _dir; }
  float fov() const { return _fov; }

protected:
  bool _use_persp {true};
  vec3 _pos {0.0f};
  vec3 _dir {0.0f, 0.0f, -1.0f};
  vec3 _up {0.0f, 1.0f, 0.0f};
  float _fov {M_PIf*0.25f};
};

mat4 proj_ortho(vec2 viewport, float znear, float zfar);

mat4 proj_persp(vec2 viewport, float znear, float zfar, float fov);

mat4 view2d(vec2 origin, vec2 center, vec2 zoom, float rot);

mat4 view3d(vec3 pos, vec3 dir, vec3 up);

using camera2d = camera<vec2>;
using camera3d = camera<vec3>;

} // namespace ntf::shogle::scene

#ifndef CAMERA_INL_HPP
#include <shogle/scene/camera.inl.hpp>
#endif
