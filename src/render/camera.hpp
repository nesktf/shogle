#pragma once

#include "math/matrix.hpp"
#include "math/complex.hpp"

#define SHOGLE_CAMERA_DECL_SETTER(signature) \
T& signature &;\
T&& signature &&

#define SHOGLE_CAMERA_COMMON_DEF_SETTER(signature, ...) \
template<typename T> \
T& impl::camera_common<T>::signature & { \
  __VA_ARGS__ \
  return static_cast<T&>(*this); \
} \
template<typename T> \
T&& impl::camera_common<T>::signature && { \
  __VA_ARGS__ \
  return static_cast<T&&>(*this); \
}

#define SHOGLE_CAMERA_DEF_SETTER(dim, signature, ...) \
template<typename T> \
T& impl::camera<dim, T>::signature & { \
  __VA_ARGS__ \
  return static_cast<T&>(*this); \
} \
template<typename T> \
T&& impl::camera<dim, T>::signature && { \
  __VA_ARGS__ \
  return static_cast<T&&>(*this); \
}

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

  using camera_common<T>::viewport;

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

  using camera_common<T>::viewport;

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


SHOGLE_CAMERA_COMMON_DEF_SETTER(znear(float znear),
  _znear = znear;
  _proj_dirty = true;
)

SHOGLE_CAMERA_COMMON_DEF_SETTER(zfar(float zfar),
  _zfar = zfar;
  _proj_dirty = true;
)

template<typename T>
mat4 impl::camera_common<T>::build_proj_ortho(vec2 viewport, float znear, float zfar) {
  return glm::ortho(
    0.0f, viewport.x,
    viewport.y, 0.0f,
    znear, zfar
  );
}

template<typename T>
mat4 impl::camera_common<T>::build_proj_persp(vec2 viewport, float znear, float zfar, float fov) {
  return glm::perspective(
    fov,
    viewport.x/viewport.y,
    znear, zfar
  );
}


SHOGLE_CAMERA_DEF_SETTER(2, pos(vec2 pos),
  _pos = pos;
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(2, pos(float x, float y),
 _pos = vec2{x, y};
 this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(2, pos(cmplx pos),
  _pos = vec2{pos.real(), pos.imag()};
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(2, zoom(vec2 zoom),
  _zoom = zoom;
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(2, zoom(float x, float y),
  _zoom = vec2{x, y};
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(2, zoom(cmplx zoom),
  _zoom = vec2{zoom.real(), zoom.imag()};
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(2, zoom(float zoom),
  _zoom = vec2{zoom, zoom};
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(2, rot(float rot),
  _rot = rot;
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(2, viewport(vec2 viewport),
  this->_viewport = viewport;
  _origin = this->_viewport*.5f;
  this->_proj_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(2, viewport(cmplx viewport),
  this->_viewport = vec2{viewport.real(), viewport.imag()};
  _origin = this->_viewport*.5f;
  this->_proj_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(2, viewport(float x, float y),
  this->_viewport = vec2{x, y};
  _origin = this->_viewport*.5f;
  this->_proj_dirty = true;
)

template<typename T>
mat4 impl::camera<2, T>::build_view(vec2 origin, vec2 pos, vec2 zoom, float rot) {
  mat4 view {1.0f};

  view = glm::translate(view, vec3{origin, 0.0f});
  view = glm::rotate(view, rot, vec3{0.0f, 0.0f, 1.0f});
  view = glm::scale(view, vec3{zoom, 1.0f});
  view = glm::translate(view, vec3{-pos, 0.0f});

  return view;
}

template<typename T>
mat4 impl::camera<2, T>::build_view(vec2 origin, vec2 pos, vec2 zoom, vec3 rot) {
  mat4 view {1.0f};

  view = glm::translate(view, vec3{origin, 0.0f});
  view = glm::rotate(view, rot.x, vec3{1.0f, 0.0f, 0.0f});
  view = glm::rotate(view, rot.y, vec3{0.0f, 1.0f, 0.0f});
  view = glm::rotate(view, rot.z, vec3{0.0f, 0.0f, 1.0f});
  view = glm::scale(view, vec3{zoom, 1.0f});
  view = glm::translate(view, vec3{-pos, 0.0f});

  return view;
}


SHOGLE_CAMERA_DEF_SETTER(3, pos(vec3 pos),
  _pos = pos;
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(3, pos(float x, float y, float z),
  _pos = vec3{x, y, z};
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(3, pos(vec2 xy, float z),
  _pos = vec3{xy.x, xy.y, z};
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(3, pos(float x, vec2 yz),
  _pos = vec3{x, yz.x, yz.y};
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(3, pos(cmplx xy, float z),
  _pos = vec3{xy.real(), xy.imag(), z};
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(3, pos(float x, cmplx yz),
  _pos = vec3{x, yz.real(), yz.imag()};
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(3, dir(vec3 dir),
  _dir = dir;
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(3, dir(float x, float y, float z),
  _dir = vec3{x, y, z};
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(3, dir(vec2 xy, float z),
  _dir = vec3{xy.x, xy.y, z};
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(3, dir(float x, vec2 yz),
  _dir = vec3{x, yz.x, yz.y};
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(3, dir(cmplx xy, float z),
  _dir = vec3{xy.real(), xy.imag(), z};
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(3, dir(float x, cmplx yz),
  _dir = vec3{x, yz.real(), yz.imag()};
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(3, up(vec3 up),
  _up = up;
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(3, up(float x, float y, float z),
  _up = vec3{x, y, z};
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(3, up(vec2 xy, float z),
  _up = vec3{xy.x, xy.y, z};
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(3, up(float x, vec2 yz),
  _up = vec3{x, yz.x, yz.y};
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(3, up(cmplx xy, float z),
  _up = vec3{xy.real(), xy.imag(), z};
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(3, up(float x, cmplx yz),
  _up = vec3{x, yz.real(), yz.imag()};
  this->_view_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(3, viewport(vec2 viewport),
  this->_viewport = viewport;
  this->_proj_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(3, viewport(cmplx viewport),
  this->_viewport = vec2{viewport.real(), viewport.imag()};
  this->_proj_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(3, viewport(float x, float y),
  this->_viewport = vec2{x, y};
  this->_proj_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(3, fov(float fov),
  _fov = fov;
  this->_proj_dirty = true;
)

SHOGLE_CAMERA_DEF_SETTER(3, proj_type(camera_proj proj),
  _proj_type = proj;
  this->_proj_dirty = true;
)

template<typename T>
mat4 impl::camera<3, T>::build_view(vec3 pos, vec3 dir, vec3 up) {
  return glm::lookAt(
    pos,
    pos + dir,
    up
  );
}


template<std::size_t dim>
void camera<dim>::force_update_proj() & {
  if constexpr (dim == 2) {
    this->_proj = this->build_proj_ortho(this->_viewport, this->_znear, this->_zfar);
  } else {
    if (this->_proj_type == camera_proj::orthographic) {
      this->_proj = this->build_proj_ortho(this->_viewport, this->_znear, this->_zfar);
    } else {
      this->_proj = this->build_proj_persp(this->_viewport, this->_znear, this->_zfar, this->_fov);
    }
  }
  this->_proj_dirty = false;
}

template<std::size_t dim>
void camera<dim>::force_update_view() & {
  if constexpr (dim == 2) {
    this->_view = this->build_view(this->_origin, this->_pos, this->_zoom, this->_rot);
  } else {
    this->_view = this->build_view(this->_pos, this->_dir, this->_up);
  }
  this->_view_dirty = false;
}

template<std::size_t dim>
void camera<dim>::force_update() & {
  force_update_proj();
  force_update_view();
}

template<std::size_t dim>
const mat4& camera<dim>::proj() & {
  if (this->_proj_dirty) {
    force_update_proj();
  }
  return this->_proj;
}

template<std::size_t dim>
const mat4& camera<dim>::view() & {
  if (this->_view_dirty) {
    force_update_view();
  }
  return this->_view;
}

template<std::size_t dim>
mat4 camera<dim>::proj() && {
  if constexpr (dim == 2) {
    return this->build_proj_ortho(this->_viewport, this->_znear, this->_zfar);
  } else {
    if (this->_proj_type == camera_proj::orthographic) {
      return this->build_proj_ortho(this->_viewport, this->_znear, this->_zfar);
    } else {
      return this->build_proj_persp(this->_viewport, this->_znear, this->_zfar, this->_fov);
    }
  }
}

template<std::size_t dim>
mat4 camera<dim>::view() && {
  if constexpr (dim == 2) {
    return this->build_view(this->_origin, this->_pos, this->_zoom, this->_rot);
  } else {
    return this->build_view(this->_pos, this->_dir, this->_up);
  }
}

template<std::size_t dim>
std::pair<const mat4&, const mat4&> camera<dim>::mat() & {
  return std::make_pair(proj(), view());
}

template<std::size_t dim>
std::pair<mat4, mat4> camera<dim>::mat() && {
  return std::make_pair(proj(), view());
}

} // namespace ntf

#undef SHOGLE_CAMERA_DECL_SETTER
#undef SHOGLE_CAMERA_DEF_SETTER
#undef SHOGLE_CAMERA_COMMON_DEF_SETTER
