#pragma once

#include <shogle/core/types.hpp>

namespace ntf {

template<size_t dim_size, typename T>
class camera_view;


template<size_t dim_size>
class camera : public camera_view<dim_size, camera<dim_size>> {
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
class camera_view<2, T> {
protected:
  camera_view(vec2 viewport);

public:
  inline T& set_pos(vec2 center);
  inline T& set_pos(float x, float y);
  inline T& set_zoom(vec2 zoom);
  inline T& set_zoom(float x, float y);
  inline T& set_zoom(float zoom);
  inline T& set_rot(float rot);

public:
  vec2 center() const { return _pos; }
  vec2 zoom() const { return _zoom; }
  float rot() const { return _rot; }

protected:
  vec2 _pos {0.0f};
  vec2 _zoom {1.0f};
  float _rot {0.0f};
  vec2 _origin;
};


template<typename T>
class camera_view<3, T> {
public:
  enum class proj_type {
    perspective,
    orthographic,
  };

protected:
  camera_view(vec2 viewport);

public:
  inline T& set_pos(vec3 pos);
  inline T& set_pos(float x, float y, float z);
  inline T& set_dir(vec3 dir);
  inline T& set_dir(float x, float y, float z);
  inline T& set_fov(float fov);
  inline T& set_proj(proj_type type);

public:
  vec3 pos() const { return _pos; }
  vec3 dir() const { return _dir; }
  vec3 up() const { return _up; }
  float fov() const { return _fov; }

protected:
  bool _use_persp {true};
  vec3 _pos {0.0f};
  vec3 _dir {0.0f, 0.0f, -1.0f};
  vec3 _up {0.0f, 1.0f, 0.0f};
  float _fov {M_PIf*0.25f};
};


inline mat4 proj_ortho(vec2 viewport, float znear, float zfar) {
  return glm::ortho(
    0.0f, viewport.x,
    viewport.y, 0.0f,
    znear, zfar
  );
}

inline mat4 proj_persp(vec2 viewport, float znear, float zfar, float fov) {
  return glm::perspective(
    fov,
    viewport.x/viewport.y,
    znear, zfar
  );
}

inline mat4 view2d(vec2 origin, vec2 pos, vec2 zoom, float rot) {
  mat4 view {1.0f};

  view = glm::translate(view, vec3{origin, 0.0f});
  view = glm::rotate(view, rot, vec3{0.0f, 0.0f, 1.0f});
  view = glm::scale(view, vec3{zoom, 1.0f});
  view = glm::translate(view, vec3{-pos, 0.0f});

  return view;
}

inline mat4 view3d(vec3 pos, vec3 dir, vec3 up) {
  return glm::lookAt(
    pos,
    pos + dir,
    up
  );
}


using camera2d = camera<2>;
using camera3d = camera<3>;


template<size_t dim_size>
camera<dim_size>::camera(vec2 viewport) : 
  camera_view<dim_size, camera<dim_size>>(viewport),
  _viewport(viewport) {
  if constexpr (dim_size == 2) {
    _znear = -10.0f;
    _zfar = 1.0f;
  }
  update();
};

template<size_t dim_size>
camera<dim_size>::camera(float w, float h) :
  camera(vec2{w, h}) {}

template<size_t dim_size>
void camera<dim_size>::update() {
  if constexpr (dim_size == 2) {
    _proj = proj_ortho(_viewport, _znear, _zfar);
    _view = view2d(this->_origin, this->_pos, this->_zoom, this->_rot);
  } else {
    _proj = this->_use_persp ?
      proj_persp(_viewport, _znear, _zfar, this->_fov) : proj_ortho(_viewport, _znear, _zfar);
    _view = view3d(this->_pos, this->_dir, this->_up);
  }
}

template<size_t dim_size>
inline auto camera<dim_size>::set_viewport(vec2 viewport) -> camera& {
  _viewport = viewport;
  if constexpr (dim_size == 2) {
    this->_origin = viewport * 0.5f;
  }
  return *this;
}

template<size_t dim_size>
inline auto camera<dim_size>::set_viewport(float w, float h) -> camera& {
  return set_viewport(vec2{w, h});
}

template<size_t dim_size>
inline auto camera<dim_size>::set_znear(float znear) -> camera& {
  _znear = znear;
  return *this;
}

template<size_t dim_size>
inline auto camera<dim_size>::set_zfar(float zfar) -> camera& {
  _zfar = zfar;
  return *this;
}


template<typename T>
camera_view<2, T>::camera_view(vec2 viewport) :
  _origin(viewport*0.5f) {}

template<typename T>
inline auto camera_view<2, T>::set_pos(vec2 center) -> T& {
  _pos = center;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto camera_view<2, T>::set_pos(float x, float y) -> T& {
  return set_pos(vec2{x, y});
}

template<typename T>
inline auto camera_view<2, T>::set_zoom(vec2 zoom) -> T& {
  _zoom = zoom;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto camera_view<2, T>::set_zoom(float x, float y) -> T& {
  return set_zoom(vec2{x, y});
}

template<typename T>
inline auto camera_view<2, T>::set_zoom(float zoom) -> T& {
  return set_zoom(vec2{zoom});
}

template<typename T>
inline auto camera_view<2, T>::set_rot(float rot) -> T& {
  _rot = rot;
  return static_cast<T&>(*this);
}


template<typename T>
camera_view<3, T>::camera_view(vec2) {}

template<typename T>
inline auto camera_view<3, T>::set_pos(vec3 pos) -> T& {
  _pos = pos;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto camera_view<3, T>::set_pos(float x, float y, float z) -> T& {
  return set_pos(vec3{x, y, z});
}

template<typename T>
inline auto camera_view<3, T>::set_dir(vec3 dir) -> T& {
  _dir = dir;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto camera_view<3, T>::set_dir(float x, float y, float z) -> T& {
  return set_dir(vec3{x, y, z});
}

template<typename T>
inline auto camera_view<3, T>::set_fov(float fov) -> T& {
  _fov = fov;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto camera_view<3, T>::set_proj(proj_type type) -> T& {
  _use_persp = (type == proj_type::perspective);
  return static_cast<T&>(*this);
}

} // namespace ntf
