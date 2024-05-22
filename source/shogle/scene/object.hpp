#pragma once

#include <shogle/math/conversions.hpp>

namespace ntf::shogle::scene {

template<size_t dim_size>
concept valid_dimension = (dim_size == 2 || dim_size == 3);

template<size_t dim_size>
requires(valid_dimension<dim_size>)
struct obj_dim;

template<>
struct obj_dim<2> { 
  using space_t = vec2;
  using rot_t = float;
  static const constexpr rot_t def_rot = 0.0f;
};

template<>
struct obj_dim<3> { 
  using space_t = vec3;
  using rot_t = quat;
  static const constexpr rot_t def_rot = quat{1.0f, vec3{0.0f}};
};


// Types
template<size_t dim_size> requires(valid_dimension<dim_size>)
class object {
public:
  using space_t = obj_dim<dim_size>::space_t;
  using rot_t = obj_dim<dim_size>::rot_t;

public:
  object();

public:
  void update_transform();

public:
  inline object& set_pos(space_t pos);
  inline object& set_scale(space_t scale);
  inline object& set_scale(float scale);
  inline object& set_rot(rot_t rot);

  template<size_t _dim = dim_size> requires(_dim == 2)
  inline object& set_pos(cmplx pos);

  template<size_t _dim = dim_size> requires(_dim == 3)
  inline object& set_rot(float ang, vec3 axis);

  template<size_t _dim = dim_size> requires(_dim == 3)
  inline object& set_rot(vec3 euler_ang);

  template<typename... coords> requires(sizeof...(coords) == dim_size)
  inline object& set_pos(coords... coord);

public:
  mat4 transform() const { return _transform; }
  bool dirty() const { return _dirty_flag; }

  space_t pos() const { return _pos; }
  space_t scale() const { return _scale; }
  rot_t rot() const { return _rot; }

  template<size_t _dim = dim_size> requires(_dim == 2)
  cmplx cpos() const { return cmplx{_pos.x, _pos.y}; }

  template<size_t _dim = dim_size> requires(_dim == 3)
  vec3 erot() const { return glm::eulerAngles(_rot); }

private:
  mat4 _transform {1.0f};
  bool _dirty_flag {false};

  space_t _pos {0.0f};
  space_t _scale {1.0f};
  rot_t _rot {obj_dim<dim_size>::def_rot};
};


// Functions
mat4 obj_transform(vec3 pos, vec3 scale, quat rot);
mat4 obj_transform(vec2 pos, vec2 scale, float rot);


// Aliases
using object2d = object<2>;
using object3d = object<3>;


// Inline definitions
template<size_t dim_size> requires(valid_dimension<dim_size>)
object<dim_size>::object() { _transform = obj_transform(_pos, _scale, _rot); };

template<size_t dim_size> requires(valid_dimension<dim_size>)
void object<dim_size>::update_transform() {
  if (_dirty_flag) {
    _transform = obj_transform(_pos, _scale, _rot);
    _dirty_flag = false;
  }
}

template<size_t dim_size> requires(valid_dimension<dim_size>)
inline auto object<dim_size>::set_pos(space_t pos) -> object& {
  _pos = pos;
  _dirty_flag = true;
  return *this;
}

template<size_t dim_size> requires(valid_dimension<dim_size>)
inline auto object<dim_size>::set_scale(space_t scale) -> object& {
  _scale = scale;
  _dirty_flag = true;
  return *this;
}

template<size_t dim_size> requires(valid_dimension<dim_size>)
inline auto object<dim_size>::set_scale(float scale) -> object& {
  _scale = space_t{scale};
  _dirty_flag = true;
  return *this;
}

template<size_t dim_size> requires(valid_dimension<dim_size>)
inline auto object<dim_size>::set_rot(rot_t rot) -> object& {
  _rot = rot;
  _dirty_flag = true;
  return *this;
}
 
template<size_t dim_size> requires(valid_dimension<dim_size>)
template<size_t _dim> requires(_dim == 2)
inline auto object<dim_size>::set_pos(cmplx pos) -> object& {
  _pos.x = pos.real();
  _pos.y = pos.imag();
  _dirty_flag = true;
  return *this;
}

template<size_t dim_size> requires(valid_dimension<dim_size>)
template<size_t _dim> requires(_dim == 3)
inline auto object<dim_size>::set_rot(float ang, vec3 axis) -> object& {
  _rot = math::axisquat(ang, axis);
  _dirty_flag = true;
  return *this;
}

template<size_t dim_size> requires(valid_dimension<dim_size>)
template<size_t _dim> requires(_dim == 3)
inline auto object<dim_size>::set_rot(vec3 euler_ang) -> object& {
  _rot = math::eulerquat(euler_ang);
  _dirty_flag = true;
  return *this;
}

template<size_t dim_size> requires(valid_dimension<dim_size>)
template<typename... coords> requires(sizeof...(coords) == dim_size)
inline auto object<dim_size>::set_pos(coords... coord) -> object& {
  _pos = space_t{coord...};
  _dirty_flag = true;
  return *this;
}

} // namespace ntf::shogle::scene
