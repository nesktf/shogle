#define CAMERA_INL_HPP
#include <shogle/scene/camera.hpp>
#undef CAMERA_INL_HPP

namespace ntf::shogle::scene {

template<typename dim_t>
camera<dim_t>::camera(vec2 viewport) : 
  camera_view<dim_t, camera<dim_t>>(viewport),
  _viewport(viewport) {
  if constexpr (std::same_as<dim_t, vec2>) {
    _znear = -10.0f;
    _zfar = 1.0f;
  }
  update();
};

template<typename dim_t>
camera<dim_t>::camera(float w, float h) :
  camera(vec2{w, h}) {}

template<typename dim_t>
void camera<dim_t>::update() {
  if constexpr (std::same_as<dim_t, vec2>) {
    _proj = proj_ortho(_viewport, _znear, _zfar);
    _view = view2d(this->_origin, this->_pos, this->_zoom, this->_rot);
  } else {
    _proj = this->_use_persp ?
      proj_persp(_viewport, _znear, _zfar, this->_fov) : proj_ortho(_viewport, _znear, _zfar);
    _view = view3d(this->_pos, this->_dir, this->_up);
  }
}

template<typename dim_t>
inline auto camera<dim_t>::set_viewport(vec2 viewport) -> camera& {
  _viewport = viewport;
  if constexpr (std::same_as<dim_t, vec2>) {
    this->_origin = viewport * 0.5f;
  }
  return *this;
}

template<typename dim_t>
inline auto camera<dim_t>::set_viewport(float w, float h) -> camera& {
  return set_viewport(vec2{w, h});
}

template<typename dim_t>
inline auto camera<dim_t>::set_znear(float znear) -> camera& {
  _znear = znear;
  return *this;
}

template<typename dim_t>
inline auto camera<dim_t>::set_zfar(float zfar) -> camera& {
  _zfar = zfar;
  return *this;
}


template<typename T>
camera_view<vec2, T>::camera_view(vec2 viewport) :
  _origin(viewport*0.5f) {}

template<typename T>
inline auto camera_view<vec2, T>::set_position(vec2 center) -> T& {
  _pos = center;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto camera_view<vec2, T>::set_position(float x, float y) -> T& {
  return set_position(vec2{x, y});
}

template<typename T>
inline auto camera_view<vec2, T>::set_zoom(vec2 zoom) -> T& {
  _zoom = zoom;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto camera_view<vec2, T>::set_zoom(float x, float y) -> T& {
  return set_zoom(vec2{x, y});
}

template<typename T>
inline auto camera_view<vec2, T>::set_zoom(float zoom) -> T& {
  return set_zoom(vec2{zoom});
}

template<typename T>
inline auto camera_view<vec2, T>::set_rotation(float rot) -> T& {
  _rot = rot;
  return static_cast<T&>(*this);
}


template<typename T>
camera_view<vec3, T>::camera_view(vec2) {}

template<typename T>
inline auto camera_view<vec3, T>::set_position(vec3 pos) -> T& {
  _pos = pos;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto camera_view<vec3, T>::set_position(float x, float y, float z) -> T& {
  return set_position(vec3{x, y, z});
}

template<typename T>
inline auto camera_view<vec3, T>::set_direction(vec3 dir) -> T& {
  _dir = dir;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto camera_view<vec3, T>::set_direction(float x, float y, float z) -> T& {
  return set_direction(vec3{x, y, z});
}

template<typename T>
inline auto camera_view<vec3, T>::set_fov(float fov) -> T& {
  _fov = fov;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto camera_view<vec3, T>::use_perspective() -> T& {
  _use_persp = true;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto camera_view<vec3, T>::use_orthographic() -> T& {
  _use_persp = false;
  return static_cast<T&>(*this);
}

} // namespace ntf::shogle::scene
