#pragma once

#include "core/types.hpp"

#include <glm/trigonometric.hpp>

namespace ntf {

inline vec3 ogl2car(const vec3& vec) {
  return {vec.z, vec.x, vec.y};
}

inline vec3 car2ogl(const vec3& vec) {
  return {vec.y, vec.z, vec.x}; 
}

inline vec3 sph2car(const float r, const float theta, const float phi) {
  return {r*glm::sin(theta)*glm::cos(phi), r*glm::sin(theta)*glm::sin(phi), r*glm::cos(theta)};
}

inline vec3 sph2ogl(const float r, const float theta, const float phi) {
  return {r*glm::sin(theta)*glm::sin(phi), r*glm::cos(theta), r*glm::sin(theta)*glm::cos(phi)};
}

} // namespace ntf
