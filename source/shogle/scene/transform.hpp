#pragma once

#include <shogle/math/conversions.hpp>

#include <vector>

namespace ntf::shogle::scene {

// Types
template<typename dim_t, typename rot_t>
class transform {
public:
  static constexpr size_t dim_size = std::same_as<dim_t, vec3> ? 3 : 2;

public:
  transform();

public:
  void update(const transform* parent = nullptr);
  void add_child(transform* child);

public:
  inline transform& set_pos(dim_t pos);

  template<size_t dim = dim_size> requires(dim == 2)
  inline transform& set_pos(cmplx pos);

  template<size_t dim = dim_size> requires(dim == 2)
  inline transform& set_pos(float x, float y);

  template<size_t dim = dim_size> requires(dim == 3)
  inline transform& set_pos(float x, float y, float z);


  inline transform& set_scale(dim_t scale);
  inline transform& set_scale(float scale);


  inline transform& set_rot(rot_t rot);

  template<size_t dim = dim_size> requires(dim == 3)
  inline transform& set_rot(float ang, vec3 axis);

  template<size_t dim = dim_size> requires(dim == 3)
  inline transform& set_rot(vec3 euler_ang);

public:
  mat4 transf() const { return _mat; }

  dim_t pos() const { return _pos; }
  dim_t scale() const { return _scale; }
  rot_t rot() const { return _rot; }

  template<size_t dim = dim_size> requires(dim == 2)
  cmplx cpos() const { return cmplx{_pos.x, _pos.y}; }

  template<size_t dim = dim_size> requires(dim == 3)
  vec3 erot() const { return glm::eulerAngles(_rot); }

private:
  mat4 _mat {1.0f};
  bool _dirty {false};

  dim_t _pos {0.0f};
  dim_t _scale {1.0f};
  rot_t _rot {};

  std::vector<transform*> _children;
};


// Functions
mat4 transf_mat(vec3 pos, vec3 scale, quat rot);
mat4 transf_mat(vec2 pos, vec2 scale, float rot);


// Aliases
using transform2d = transform<vec2, float>;
using transform3d = transform<vec3, quat>;


// Inline definitions
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
    child->_dirty |= _dirty;
    child->update(this);
  }
  _dirty = false;
}

template<typename dim_t, typename rot_t>
void transform<dim_t, rot_t>::add_child(transform* child) {
  _children.push_back(child);
  child->_dirty = true;
  child->update(this);
}

template<typename dim_t, typename rot_t>
inline auto transform<dim_t, rot_t>::set_pos(dim_t pos) -> transform& {
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
inline auto transform<dim_t, rot_t>::set_rot(rot_t rot) -> transform& {
  _rot = rot;
  _dirty = true;
  return *this;
}
 
template<typename dim_t, typename rot_t>
template<size_t dim> requires(dim == 2)
inline auto transform<dim_t, rot_t>::set_pos(cmplx pos) -> transform& {
  _pos.x = pos.real();
  _pos.y = pos.imag();
  _dirty = true;
  return *this;
}

template<typename dim_t, typename rot_t>
template<size_t dim> requires(dim == 3)
inline auto transform<dim_t, rot_t>::set_rot(float ang, vec3 axis) -> transform& {
  _rot = math::axisquat(ang, axis);
  _dirty = true;
  return *this;
}

template<typename dim_t, typename rot_t>
template<size_t dim> requires(dim == 3)
inline auto transform<dim_t, rot_t>::set_rot(vec3 euler_ang) -> transform& {
  _rot = math::eulerquat(euler_ang);
  _dirty = true;
  return *this;
}

template<typename dim_t, typename rot_t>
template<size_t dim> requires(dim == 2)
inline auto transform<dim_t, rot_t>::set_pos(float x, float y) -> transform& {
  _pos = dim_t{x, y};
  _dirty = true;
  return *this;
}

template<typename dim_t, typename rot_t>
template<size_t dim> requires(dim == 3)
inline auto transform<dim_t, rot_t>::set_pos(float x, float y, float z) -> transform& {
  _pos = dim_t{x, y, z};
  _dirty = true;
  return *this;
}

} // namespace ntf::shogle::scene
