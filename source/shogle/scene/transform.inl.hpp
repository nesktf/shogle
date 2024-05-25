#define TRANSFORM_INL_HPP
#include <shogle/scene/transform.hpp>
#undef TRANSFORM_INL_HPP

namespace ntf::shogle::scene {

template<typename dim_t, typename rot_t>
transform<dim_t, rot_t>::transform() { 
  if constexpr (dim_size == 3) {
    _rot = quat{1.0f, vec3{0.0f}};
  }
  _mat = transf_mat(_pos, _scale, _rot); 
}

template<typename dim_t, typename rot_t>
void transform<dim_t, rot_t>::update(const transform* parent) {
  if (_dirty) {
    if (parent) {
      _mat = parent->_mat*transf_mat(_pos, _scale, _rot);
    } else {
      _mat = transf_mat(_pos, _scale, _rot);
    }
  }
  for (auto* child : _children) {
    child->_dirty |= _dirty; // if dirty, force child to update
    child->update(this);
  }
  _dirty = false;
}

template<typename dim_t, typename rot_t>
void transform<dim_t, rot_t>::add_child(transform* child) {
  _children.push_back(child);
  child->_dirty = true; // force child to update
  child->update(this);
}

template<typename dim_t, typename rot_t>
inline auto transform<dim_t, rot_t>::set_position(dim_t pos) -> transform& {
  _pos = pos;
  _dirty = true;
  return *this;
}

template<typename dim_t, typename rot_t>
inline auto transform<dim_t, rot_t>::set_scale(dim_t scale) -> transform& {
  _scale = scale;
  _dirty = true;
  return *this;
}

template<typename dim_t, typename rot_t>
inline auto transform<dim_t, rot_t>::set_scale(float scale) -> transform& {
  _scale = dim_t{scale};
  _dirty = true;
  return *this;
}

template<typename dim_t, typename rot_t>
inline auto transform<dim_t, rot_t>::set_rotation(rot_t rot) -> transform& {
  _rot = rot;
  _dirty = true;
  return *this;
}
 
template<typename dim_t, typename rot_t>
inline auto transform<dim_t, rot_t>::set_position(cmplx pos) -> transform& 
  requires(dim_size == 2) {
  _pos.x = pos.real();
  _pos.y = pos.imag();
  _dirty = true;
  return *this;
}

template<typename dim_t, typename rot_t>
inline auto transform<dim_t, rot_t>::set_rotation(float ang, vec3 axis) -> transform& 
  requires(dim_size == 3) {
  _rot = math::axisquat(ang, axis);
  _dirty = true;
  return *this;
}

template<typename dim_t, typename rot_t>
inline auto transform<dim_t, rot_t>::set_rotation(vec3 euler_ang) -> transform& 
  requires(dim_size == 3) {
  _rot = math::eulerquat(euler_ang);
  _dirty = true;
  return *this;
}

template<typename dim_t, typename rot_t>
inline auto transform<dim_t, rot_t>::set_position(float x, float y) -> transform& 
  requires(dim_size == 2) {
  _pos = dim_t{x, y};
  _dirty = true;
  return *this;
}

template<typename dim_t, typename rot_t>
inline auto transform<dim_t, rot_t>::set_position(float x, float y, float z) -> transform& 
  requires(dim_size == 3) {
  _pos = dim_t{x, y, z};
  _dirty = true;
  return *this;
}

} // namespace ntf::shogle::scene
