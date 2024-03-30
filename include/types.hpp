#pragma once

#include <glm/vec3.hpp>

#include <functional>

namespace ntf::shogle {

template<typename T>
using ref = std::reference_wrapper<T>;

template<typename T>
using cref = std::reference_wrapper<const T>;

struct TransformData {
  glm::vec3 pos {0.0f};
  glm::vec3 scale {0.0f};
  glm::vec3 rot {0.0f};
};


} // namespace ntf::shogle
