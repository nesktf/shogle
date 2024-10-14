#pragma once

#include <shogle/math/alg.hpp>

#define SHOGLE_CAMERA_DECL_SETTER(signature) \
T& signature &;\
T&& signature &&

namespace ntf {

enum class camera_proj {
  orthographic = 0,
  perspective,
};

namespace impl {

template<typename T>
class camera_common {
public:
  camera_common() = default;

public:
  SHOGLE_CAMERA_DECL_SETTER(znear(float znear));
  SHOGLE_CAMERA_DECL_SETTER(zfar(float zfar));

public:
  float znear() const { return _znear; }
  float zfar() const { return _zfar; }
  vec2 viewport() const { return _viewport; }

  bool view_dirty() const { return _view_dirty; }
  bool proj_dirty() const { return _proj_dirty; }

public:
  static mat4 build_proj_ortho(vec2 viewport, float znear, float zfar);
  static mat4 build_proj_persp(vec2 viewport, float znear, float zfar, float fov);

protected:
  mat4 _proj, _view;
  float _znear{.1f}, _zfar{100.f};
  vec2 _viewport{1.f, 1.f};
  bool _view_dirty{true}, _proj_dirty{true};
};


template<std::size_t dim, typename T>
class camera;


template<typename T>
class camera<2, T> : public camera_common<T> {
public:
  camera() = default;

public:
  SHOGLE_CAMERA_DECL_SETTER(pos(vec2 pos));
  SHOGLE_CAMERA_DECL_SETTER(pos(float x, float y));
  SHOGLE_CAMERA_DECL_SETTER(pos(cmplx pos));

  SHOGLE_CAMERA_DECL_SETTER(zoom(vec2 zoom));
  SHOGLE_CAMERA_DECL_SETTER(zoom(float x, float y));
  SHOGLE_CAMERA_DECL_SETTER(zoom(cmplx zoom));
  SHOGLE_CAMERA_DECL_SETTER(zoom(float zoom));

  SHOGLE_CAMERA_DECL_SETTER(rot(float rot));

  SHOGLE_CAMERA_DECL_SETTER(viewport(vec2 viewport));
  SHOGLE_CAMERA_DECL_SETTER(viewport(cmplx viewport));
  SHOGLE_CAMERA_DECL_SETTER(viewport(float x, float y));

public:
  vec2 pos() const { return _pos; }
  vec2 zoom() const { return _zoom; }
  float rot() const { return _rot; }

  cmplx cpos() const { return cmplx{_pos.x, _pos.y}; }
  float zoom_x() const { return _zoom.x; }
  float zoom_y() const { return _zoom.y;}

public:
  static mat4 build_view(vec2 origin, vec2 pos, vec2 zoom, float rot);
  static mat4 build_view(vec2 origin, vec2 pos, vec2 zoom, vec3 rot);

protected:
  vec2 _pos{0.f};
  vec2 _zoom{1.f};
  float _rot{0.f};
  vec2 _origin{0.f};
};


template<typename T>
class camera<3, T> : public camera_common<T> {
public:
  camera() = default;

public:
  SHOGLE_CAMERA_DECL_SETTER(pos(vec3 pos));
  SHOGLE_CAMERA_DECL_SETTER(pos(float x, float y, float z));
  SHOGLE_CAMERA_DECL_SETTER(pos(vec2 xy, float z));
  SHOGLE_CAMERA_DECL_SETTER(pos(float x, vec2 yz));
  SHOGLE_CAMERA_DECL_SETTER(pos(cmplx xy, float z));
  SHOGLE_CAMERA_DECL_SETTER(pos(float x, cmplx yz));

  SHOGLE_CAMERA_DECL_SETTER(dir(vec3 dir));
  SHOGLE_CAMERA_DECL_SETTER(dir(float x, float y, float z));
  SHOGLE_CAMERA_DECL_SETTER(dir(vec2 xy, float z));
  SHOGLE_CAMERA_DECL_SETTER(dir(float x, vec2 yz));
  SHOGLE_CAMERA_DECL_SETTER(dir(cmplx xy, float z));
  SHOGLE_CAMERA_DECL_SETTER(dir(float x, cmplx yz));

  SHOGLE_CAMERA_DECL_SETTER(up(vec3 up));
  SHOGLE_CAMERA_DECL_SETTER(up(float x, float y, float z));
  SHOGLE_CAMERA_DECL_SETTER(up(vec2 xy, float z));
  SHOGLE_CAMERA_DECL_SETTER(up(float x, vec2 yz));
  SHOGLE_CAMERA_DECL_SETTER(up(cmplx xy, float z));
  SHOGLE_CAMERA_DECL_SETTER(up(float x, cmplx yz));

  SHOGLE_CAMERA_DECL_SETTER(viewport(vec2 viewport));
  SHOGLE_CAMERA_DECL_SETTER(viewport(cmplx viewport));
  SHOGLE_CAMERA_DECL_SETTER(viewport(float x, float y));

  SHOGLE_CAMERA_DECL_SETTER(fov(float fov));
  SHOGLE_CAMERA_DECL_SETTER(proj_type(camera_proj proj));

public:
  vec3 pos() const { return _pos; }
  vec3 dir() const { return _dir; }
  vec3 up() const { return _up; }
  float fov() const { return _fov; }
  camera_proj proj_type() const { return _proj_type; }

public:
  static mat4 build_view(vec3 pos, vec3 dir, vec3 up);

protected:
  vec3 _pos{0.f};
  vec3 _dir {0.0f, 0.0f, -1.0f};
  vec3 _up {0.0f, 1.0f, 0.0f};
  float _fov {M_PIf*0.25f};
  camera_proj _proj_type{camera_proj::perspective};
};

} // namespace impl

template<std::size_t dim>
class camera : public impl::camera<dim, ::ntf::camera<dim>> {
public:
  camera() = default;

public:
  void force_update_proj() &;
  void force_update_view() &;
  void force_update() &;

public:
  const mat4& proj() &; // Not const
  const mat4& view() &; // Not const
  mat4 proj() &&;
  mat4 view() &&;

  std::pair<const mat4&, const mat4&> mat() &;
  std::pair<mat4, mat4> mat() &&;
};

using camera2d = camera<2>;
using camera3d = camera<3>;

} // namespace ntf

#undef SHOGLE_CAMERA_DECL_SETTER

#ifndef SHOGLE_CAMERA_INL
#include <shogle/scene/camera.inl>
#endif
