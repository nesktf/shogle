#define SHOGLE_MATH_COMMON_INL
#include <shogle/math/common.hpp>
#undef SHOGLE_MATH_COMMON_INL

namespace shogle::math {

template<numeric_type T>
SHOGLE_MATH_DEF T radians(T deg) noexcept {
  return deg * static_cast<T>(0.01745329251994329576923690768489);
}

template<typename T>
SHOGLE_MATH_DEF T degrees(T rad) {
  return rad * static_cast<T>(57.295779513082320876798154814105);
}

template<numeric_type TL, numeric_type TR>
SHOGLE_MATH_DEF auto periodic_add(const TL& a, const TR& b, decltype(a + b) min,
                                  decltype(a + b) max) noexcept {
  auto res = a + b;
  auto range = max - min;
  while (res >= max)
    res -= range;
  while (res < min)
    res += range;
  return res;
}

template<std::floating_point T>
SHOGLE_MATH_DEF bool fequal(T a, T b) noexcept {
  return (std::fabs(a - b) <=
          std::numeric_limits<T>::epsilon() * std::fmax(std::fabs(a), std::fabs(b)));
}

template<numeric_type T>
SHOGLE_MATH_DEF T epsilon_err(T a, T b) noexcept {
  return std::abs((b - a) / b);
}

} // namespace shogle::math
