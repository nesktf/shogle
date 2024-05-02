#pragma once

#include <shogle/core/types.hpp>

#include <shogle/scene/entity.hpp>

#define PI M_PIf

namespace ntf {

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
    quat{glm::cos(rot.x*0.5f), glm::sin(rot.x*0.5f)*vec3{1.0f, 0.0f, 0.0f}} *
    quat{glm::cos(rot.y*0.5f), glm::sin(rot.y*0.5f)*vec3{0.0f, 1.0f, 0.0f}} *
    quat{glm::cos(rot.z*0.5f), glm::sin(rot.z*0.5f)*vec3{0.0f, 0.0f, 1.0f}};
};


// entity manipulation
template<typename T>
inline void rotate_entity(T& obj, float ang, vec3 axis) {
  obj.rot = quat{glm::cos(ang*0.5f), glm::sin(ang*0.5f)*axis};
}
template<typename T>
inline void rotate_entity(T& obj, quat rot) {
  obj.rot = rot;
}
template<typename T>
inline void rotate_entity(T& obj, vec3 rot) {
  obj.rot = euler2quat(rot);
}
inline void rotate_entity(entity2D& obj, float ang) {
  rotate_entity(obj, -ang, vec3{0.0f, 0.0f, 1.0f});
}

template<typename T, typename dim_t>
inline void move_entity(T& obj, dim_t pos) {
  obj.pos = pos;
}

template<typename T, typename dim_t>
inline void scale_entity(T& obj, dim_t scale) {
  obj.scale = scale;
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


} // namespace ntf
