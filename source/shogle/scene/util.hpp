#pragma once

#include <shogle/core/types.hpp>

#include <shogle/scene/entity.hpp>

#define PI M_PIf

#define I cmplx{0.0f, 1.0f}

namespace ntf {

inline quat axis_quat(float ang, vec3 axis) {
  return quat{glm::cos(ang*0.5f), glm::sin(ang*0.5f)*axis};
}

// conversions
inline vec3 ogl2car(vec3 v) { return {v.z, v.x, v.y}; }

inline vec3 car2ogl(vec3 v) { return {v.y, v.z, v.x}; }

inline vec3 sph2car(float r, float theta, float phi) {
  return r*vec3{glm::sin(theta)*glm::cos(phi), glm::sin(theta)*glm::sin(phi), glm::cos(theta)};
}

inline vec3 sph2ogl(float r, float theta, float phi) {
  return car2ogl(sph2car(r, theta, phi));
}

inline quat euler2quat(vec3 rot) {
  return
    axis_quat(rot.x, vec3{1.0f, 0.0f, 0.0f}) *
    axis_quat(rot.y, vec3{0.0f, 1.0f, 0.0f}) *
    axis_quat(rot.z, vec3{0.0f, 0.0f, 1.0f});
};

inline vec2 cmplx2vec(cmplx x) {
  return vec2{x.real(), x.imag()};
}

// misc
template<typename TL, typename TR>
constexpr auto periodic_add(TL a, TR b, decltype(a+b) min, decltype(a+b) max) {
  auto res = a + b;
  auto range = max-min;
  while (res >= max) res -= range;
  while (res <  min) res += range;
  return res;
}

inline vec2 expi(float ang) {
  return vec2{glm::cos(ang), glm::sin(ang)};
}


// entity manip
inline void set_pos(entity3d& obj, vec3 pos) {
  obj.pos = pos;
}

inline void set_pos(entity2d& obj, vec2 pos) {
  obj.pos = pos;
}

template<typename T>
inline void scale(T& obj, float scale) {
  obj.scale *= scale;
}

inline void scale(entity3d& obj, vec3 scale) {
  obj.scale *= scale;
}

inline void scale(entity2d& obj, vec2 scale) {
  obj.scale *= scale;
}

inline void set_scale(entity3d& obj, vec3 scale) {
  obj.scale = scale;
}

inline void set_scale(entity2d& obj, vec2 scale) {
  obj.scale = scale;
}

inline void rotate(entity3d& obj, float ang, vec3 axis) {
  obj.rot *= axis_quat(ang, axis);
}

inline void rotate(entity2d& obj, float ang) {
  obj.rot *= axis_quat(-ang, vec3{0.0f, 0.0f, 1.0f});
}

inline void set_rotation(entity3d& obj, float ang, vec3 axis) {
  obj.rot = axis_quat(ang, axis);
}

inline void set_rotation(entity3d& obj, vec3 euler_ang) {
  obj.rot = euler2quat(euler_ang);
}

inline void set_rotation(entity2d& obj, float ang) {
  obj.rot = axis_quat(-ang, vec3{0.0f, 0.0f, 1.0f});
}

} // namespace ntf
