#pragma once

#include "core.hpp"
#include "stl/common.hpp"

#ifdef SHOGLE_USE_GLM
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#endif

#include <cmath>

namespace ntf {

constexpr inline float32 rad(float32 deg) {
#if SHOGLE_USE_GLM
  return glm::radians(deg);
#endif
}

constexpr inline float32 deg(float32 rad) {
#if SHOGLE_USE_GLM
  return glm::degrees(rad);
#endif
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
constexpr bool equal(T f1, T f2) { 
  return (std::fabs(f1 - f2) <= std::numeric_limits<T>::epsilon() * std::fmax(std::fabs(f1),
                                                                              std::fabs(f2)));
}

template<typename T>
constexpr T epserr(const T& a, const T& b) {
  return std::abs((b-a)/b);
}

} // namespace ntf
