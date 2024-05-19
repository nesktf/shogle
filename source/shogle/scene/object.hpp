#pragma once

#include <shogle/math/conversions.hpp>

namespace ntf::shogle::scene {

mat4 gen_transform(vec3 pos, vec3 scale, quat rot);
mat4 gen_transform(vec2 pos, vec2 scale, float rot);

template<size_t dim_size>
requires(dim_size == 2 || dim_size == 3)
struct dim_t;

template<>
struct dim_t<2> { 
  using space_t = vec2;
  using rot_t = float;
  static const constexpr rot_t def_rot = 0.0f;
};

template<>
struct dim_t<3> { 
  using space_t = vec3;
  using rot_t = quat;
  static const constexpr rot_t def_rot = quat{1.0f, vec3{0.0f}};
};

template<size_t dim_size>
requires(dim_size == 2 || dim_size == 3)
class object {
public:
  using space_t = dim_t<dim_size>::space_t;
  using rot_t = dim_t<dim_size>::rot_t;

public:
  object() { _transform = gen_transform(_pos, _scale, _rot); };

public:
  void update() {
    if (_dirty_flag) {
      _transform = gen_transform(_pos, _scale, _rot);
      _dirty_flag = false;
    }
  }

public:
  object& set_pos(space_t pos) {
    _pos = pos;
    _dirty_flag = true;
    return *this;
  };

  template<size_t _dim = dim_size>
  requires(_dim == 2)
  object& set_pos(cmplx pos) {
    _pos.x = pos.real();
    _pos.y = pos.imag();
    _dirty_flag = true;
    return *this;
  }

  object& set_scale(space_t scale) {
    _scale = scale;
    _dirty_flag = true;
    return *this;
  }

  object& set_rot(rot_t rot) {
    _rot = rot;
    _dirty_flag = true;
    return *this;
  }

  template<size_t _dim = dim_size>
  requires(_dim == 3)
  object& set_rot(float ang, vec3 axis) {
    _rot = math::axis_quat(ang, axis);
    _dirty_flag = true;
    return *this;
  }

  template<size_t _dim = dim_size>
  requires(_dim == 3)
  object& set_rot(vec3 euler_ang) {
    _rot = math::euler2quat(euler_ang);
    _dirty_flag = true;
    return *this;
  }

public:
  mat4 transform() const { return _transform; }
  bool dirty() const { return _dirty_flag; }

  space_t pos() const { return _pos; }
  space_t scale() const { return _scale; }
  rot_t rot() const { return _rot; }

  template<size_t _dim = dim_size>
  requires(_dim == 2)
  cmplx cpos() const { return cmplx{_pos.x, _pos.y}; }

  template<size_t _dim = dim_size>
  requires(_dim == 3)
  vec3 erot() const { return glm::eulerAngles(_rot); }

private:
  mat4 _transform {1.0f};
  bool _dirty_flag {false};

  space_t _pos {0.0f};
  space_t _scale {1.0f};
  rot_t _rot {dim_t<dim_size>::def_rot};
};

using object2d = object<2>;
using object3d = object<3>;

} // namespace ntf::shogle::scene
