#pragma once

#include <shogle/core/types.hpp>

namespace ntf::math {

inline vec3 ogl2car(vec3 v) { return {v.z, v.x, v.y}; }

inline vec3 car2ogl(vec3 v) { return {v.y, v.z, v.x}; }

inline vec3 sph2car(float rho, float theta, float phi) {
  return rho*vec3{glm::sin(theta)*glm::cos(phi), glm::sin(theta)*glm::sin(phi), glm::cos(theta)};
}

inline vec3 sph2ogl(float rho, float theta, float phi) {
  return car2ogl(sph2car(rho, theta, phi));
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

} // namespace ntf::math
