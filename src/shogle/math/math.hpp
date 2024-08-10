#pragma once

#include <shogle/core/types.hpp>

#include <cmath>
#include <limits>

namespace ntf {

inline vec3 carcoord(vec3 v) { return {v.z, v.x, v.y}; }

inline vec3 glcoord(vec3 v) { return {v.y, v.z, v.x}; }

inline vec3 carcoord(float rho, float theta, float phi) {
  return rho*vec3{glm::sin(theta)*glm::cos(phi), glm::sin(theta)*glm::sin(phi), glm::cos(theta)};
}

inline vec3 glcoord(float rho, float theta, float phi) {
  return glcoord(carcoord(rho, theta, phi));
}

inline quat axisquat(float ang, vec3 axis) {
  return quat{glm::cos(ang*0.5f), glm::sin(ang*0.5f)*axis};
}

inline quat eulerquat(vec3 rot) {
  return
    axisquat(rot.x, vec3{1.0f, 0.0f, 0.0f}) *
    axisquat(rot.y, vec3{0.0f, 1.0f, 0.0f}) *
    axisquat(rot.z, vec3{0.0f, 0.0f, 1.0f});
};

inline vec2 expiv(float theta) {
  return vec2{glm::cos(theta), glm::sin(theta)};
}

inline cmplx expic(float theta) {
  return cmplx{glm::cos(theta), glm::sin(theta)};
}

inline float rad(float deg) {
  return glm::radians(deg);
}

inline float deg(float rad) {
  return glm::degrees(rad);
}

template<typename TL, typename TR>
constexpr auto periodic_add(TL a, TR b, decltype(a+b) min, decltype(a+b) max) {
  auto res = a + b;
  auto range = max-min;
  while (res >= max) res -= range;
  while (res <  min) res += range;
  return res;
}

template<typename T>
inline bool equal(T f1, T f2) { 
  return (std::fabs(f1 - f2) <= std::numeric_limits<T>::epsilon() * std::fmax(std::fabs(f1), std::fabs(f2)));
}

} // namespace ntf
