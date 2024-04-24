#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

#include <memory>
#include <complex>

#include <sys/types.h>

namespace ntf {

// Might replace glm at some point (?)
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using mat3 = glm::mat3;

using cmplx = std::complex<float>;
using quat = glm::quat;

using color = vec4;
using color4 = vec4;
using color3 = vec3;

using path_t = std::string;

template<typename T>
using uptr = std::unique_ptr<T>;

template<typename T, typename... Args>
inline T* make_ptr(Args&&... args) {
  return new T{std::forward<Args>(args)...};
}

template<typename T, typename... Args>
inline uptr<T> make_uptr(Args&&... args) {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

inline quat axis_quat(float ang, vec3 axis) {
  return quat{ang*0.5f, ang*0.5f*axis};
}

} // namespace ntf
