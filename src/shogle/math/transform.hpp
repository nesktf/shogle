#pragma once

#include <shogle/math/math.hpp>

#include <vector>

namespace ntf {

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


inline mat4 transf_mat(vec3 pos, vec3 scale, quat rot) {
  mat4 model{1.0f};

  model = glm::translate(model, pos);
  model*= glm::mat4_cast(rot);
  model = glm::scale(model, scale);

  return model;
}

inline mat4 transf_mat(vec2 pos, vec2 scale, float rot) {
  mat4 model{1.0f};

  model = glm::translate(model, vec3{pos, 0.0f});
  model = glm::rotate(model, rot, vec3{0.0f, 0.0f, 1.0f});
  model = glm::scale(model, vec3{scale, 1.0f});

  return model;
}


using transform2d = transform<2>;
using transform3d = transform<3>;


template<size_t dim_size>
transform<dim_size>::transform() { 
  _mat = transf_mat(this->_pos, this->_scale, this->_rot); // Init matrix
}

template<size_t dim_size>
const mat4& transform<dim_size>::mat() {
  if (this->_dirty) {
    force_update();
  }
  return _mat;
}

template<size_t dim_size>
void transform<dim_size>::force_update() {
  if (_parent) {
    _mat = _parent->_mat*transf_mat(this->_pos, this->_scale, this->_rot);
  } else {
    _mat = transf_mat(this->_pos, this->_scale, this->_rot);
  }
  for (auto* child : _children) {
    child->force_update();
  }
  this->_dirty = false;
}

template<size_t dim_size>
void transform<dim_size>::add_child(transform* child) {
  _children.push_back(child);
  child->_parent = this;
  child->_dirty = true; // force child to update
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
  _rot = axisquat(ang, axis);
  _dirty = true;
  return static_cast<T&>(*this);
}

template<typename T>
inline auto transf_impl<3, T>::set_rot(vec3 euler_ang) -> T& {
  _rot = eulerquat(euler_ang);
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

} // namespace ntf::shogle

