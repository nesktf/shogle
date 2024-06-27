#pragma once

#include <shogle/math/conversions.hpp>

#include <vector>

namespace ntf::shogle {

template<size_t dim_size, typename T>
class transf_impl;


template<size_t dim_size>
class transform : public transf_impl<dim_size, transform<dim_size>> {
public:
  transform();

public:
  void add_child(transform* child);
  void force_update();
  const mat4& mat();

private:
  mat4 _mat {1.0f};

  transform* _parent {nullptr};
  std::vector<transform*> _children;
};


template<typename T>
class transf_impl<3, T> {
protected:
  transf_impl() = default;

public:
  inline T& set_pos(vec3 pos);
  inline T& set_pos(float x, float y, float z);
  inline T& set_scale(vec3 scale);
  inline T& set_scale(float scale);
  inline T& set_rot(quat rot);
  inline T& set_rot(float ang, vec3 axis);
  inline T& set_rot(vec3 euler_ang);

public:
  inline vec3 pos() const { return _pos; }
  inline vec3 scale() const { return _scale; }
  inline quat rot() const { return _rot; }
  inline vec3 erot() const { return glm::eulerAngles(_rot); }

protected:
  bool _dirty {false};
  vec3 _pos {0.0f};
  vec3 _scale {1.0f};
  quat _rot {1.0f, vec3{0.0f}};
};


template<typename T>
class transf_impl<2, T> {
protected:
  transf_impl() = default;
  
public:
  inline T& set_pos(vec2 pos);
  inline T& set_pos(float x, float y);
  inline T& set_pos(cmplx pos);
  inline T& set_scale(vec2 scale);
  inline T& set_scale(float scale);
  inline T& set_rot(float rot);

public:
  inline vec2 pos() const { return _pos; }
  inline cmplx cpos() const { return cmplx{_pos.x, _pos.y}; }
  inline vec2 scale() const { return _scale; }
  inline float rot() const { return _rot; }

protected:
  bool _dirty {false};
  vec2 _pos {0.0f};
  vec2 _scale {1.0f};
  float _rot {0.0f};
};


mat4 transf_mat(vec3 pos, vec3 scale, quat rot);
mat4 transf_mat(vec2 pos, vec2 scale, float rot);


using transform2d = transform<2>;
using transform3d = transform<3>;

} // namespace ntf::shogle

#ifndef TRANSFORM_INL_HPP
#include <shogle/scene/transform.inl.hpp>
#endif
