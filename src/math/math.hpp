#pragma once

#include <ntfstl/types.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>

namespace ntf {

constexpr inline f32 rad(f32 deg) {
  return glm::radians(deg);
}

constexpr inline f32 deg(f32 rad) {
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
constexpr bool equal(T f1, T f2) { 
  return (std::fabs(f1 - f2) <= std::numeric_limits<T>::epsilon() * std::fmax(std::fabs(f1),
                                                                              std::fabs(f2)));
}

template<typename T>
constexpr T epserr(const T& a, const T& b) {
  return std::abs((b-a)/b);
}

} // namespace ntf
