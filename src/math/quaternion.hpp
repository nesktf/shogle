#pragma once

#include "./matrix.hpp"

#if SHOGLE_USE_GLM
#include <glm/gtc/quaternion.hpp>
#endif

namespace ntf {

#if SHOGLE_USE_GLM
template<typename T>
using qua = glm::qua<T>;
#endif

using quat = qua<float32>;
using dquat = qua<float64>;

constexpr inline quat axisquat(float ang, vec3 axis) {
#if SHOGLE_USE_GLM
  return quat{glm::cos(ang*0.5f), glm::sin(ang*0.5f)*axis};
#endif
}

constexpr inline quat eulerquat(vec3 rot) {
  return
    axisquat(rot.x, vec3{1.0f, 0.0f, 0.0f}) *
    axisquat(rot.y, vec3{0.0f, 1.0f, 0.0f}) *
    axisquat(rot.z, vec3{0.0f, 0.0f, 1.0f});
};

} // namespace ntf
