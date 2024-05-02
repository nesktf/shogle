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
