#pragma once

#include <glm/trigonometric.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace ntf {

inline glm::vec3 ogl2car(const glm::vec3& vec) {
  return {vec.z, vec.x, vec.y};
}

inline glm::vec3 car2ogl(const glm::vec3& vec) {
  return {vec.y, vec.z, vec.x}; 
}

inline glm::vec3 sph2car(const float r, const float theta, const float phi) {
  return {r*glm::sin(theta)*glm::cos(phi), r*glm::sin(theta)*glm::sin(phi), r*glm::cos(theta)};
}

inline glm::vec3 sph2ogl(const float r, const float theta, const float phi) {
  return {r*glm::sin(theta)*glm::sin(phi), r*glm::cos(theta), r*glm::sin(theta)*glm::cos(phi)};
}

} // namespace ntf
