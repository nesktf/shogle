#define CAMERA_INL_HPP
#include <shogle/scene/camera.hpp>
#undef CAMERA_INL_HPP

namespace ntf::shogle::scene {

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
inline auto camera_view<3, T>::set_direction(vec3 dir) -> T& {
  _dir = dir;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto camera_view<3, T>::set_direction(float x, float y, float z) -> T& {
  return set_direction(vec3{x, y, z});
}

template<typename T>
inline auto camera_view<3, T>::set_fov(float fov) -> T& {
  _fov = fov;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto camera_view<3, T>::set_projection(proj_type type) -> T& {
  _use_persp = (type == proj_type::perspective);
  return static_cast<T&>(*this);
}

} // namespace ntf::shogle::scene
