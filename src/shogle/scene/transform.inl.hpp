#define TRANSFORM_INL_HPP
#include <shogle/scene/transform.hpp>
#undef TRANSFORM_INL_HPP

namespace ntf::shogle::scene {

template<size_t dim_size>
transform<dim_size>::transform() { 
  _mat = transf_mat(this->_pos, this->_scale, this->_rot); // Init matrix
}

template<size_t dim_size>
void transform<dim_size>::update(const transform* parent) {
  if (this->_dirty) {
    if (parent) {
      _mat = parent->_mat*transf_mat(this->_pos, this->_scale, this->_rot);
    } else {
      _mat = transf_mat(this->_pos, this->_scale, this->_rot);
    }
  }
  for (auto* child : _children) {
    child->_dirty |= this->_dirty; // if dirty, force child to update
    child->update(this);
  }
  this->_dirty = false;
}

template<size_t dim_size>
void transform<dim_size>::add_child(transform* child) {
  _children.push_back(child);
  child->_dirty = true; // force child to update
  child->update(this);
}


template<typename T>
inline auto transf_impl<3, T>::set_pos(vec3 pos) -> T& {
  _pos = pos;
  _dirty = true;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto transf_impl<3, T>::set_pos(float x, float y, float z) -> T& {
  _pos = vec3{x, y, z};
  _dirty = true;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto transf_impl<3, T>::set_scale(vec3 scale) -> T& {
  _scale = scale;
  _dirty = true;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto transf_impl<3, T>::set_scale(float scale) -> T& {
  _scale = vec3{scale};
  _dirty = true;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto transf_impl<3, T>::set_rot(quat rot) -> T& {
  _rot = rot;
  _dirty = true;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto transf_impl<3, T>::set_rot(float ang, vec3 axis) -> T& {
  _rot = math::axisquat(ang, axis);
  _dirty = true;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto transf_impl<3, T>::set_rot(vec3 euler_ang) -> T& {
  _rot = math::eulerquat(euler_ang);
  _dirty = true;
  return static_cast<T&>(*this);
}


template<typename T>
inline auto transf_impl<2, T>::set_pos(vec2 pos) -> T& {
  _pos = pos;
  _dirty = true;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto transf_impl<2, T>::set_pos(float x, float y) -> T& {
  _pos = vec2{x, y};
  _dirty = true;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto transf_impl<2, T>::set_pos(cmplx pos) -> T& {
  _pos = vec2{pos.real(), pos.imag()};
  _dirty = true;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto transf_impl<2, T>::set_scale(vec2 scale) -> T& {
  _scale = scale;
  _dirty = true;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto transf_impl<2, T>::set_scale(float scale) -> T& {
  _scale = vec2{scale};
  _dirty = true;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto transf_impl<2, T>::set_rot(float rot) -> T& {
  _rot = rot;
  _dirty = true;
  return static_cast<T&>(*this);
}

} // namespace ntf::shogle::scene
