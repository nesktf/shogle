#define SHOGLE_MATH_COMMON_INL
#include <shogle/math/common.hpp>
#undef SHOGLE_MATH_COMMON_INL

namespace shogle::math {

template<numeric_type T>
SHOGLE_MATH_DEF T sqrt(T x) noexcept {
  return std::sqrt(x);
}

template<numeric_type T>
SHOGLE_MATH_DEF T rsqrt(T x) noexcept {
  return T(1) / std::sqrt(x);
}

SHOGLE_MATH_DEF f32 qrsqrt(f32 x) noexcept {
  const i32 shifted_x = 0x5F3759DF - (std::bit_cast<i32>(x) >> 1); // what the fuck?
  f32 y = std::bit_cast<f32>(shifted_x);
  y = y * (1.5f - (x * y * y * .5f)); // 1st iteration
  // y = y * (1.5f - (x * y * y * .5f)); // 2nd iteration
  return y;
}

template<numeric_type T>
SHOGLE_MATH_DEF T cos(T x) noexcept {
  return std::cos(x);
}

template<numeric_type T>
SHOGLE_MATH_DEF T sin(T x) noexcept {
  return std::sin(x);
}

template<numeric_type T>
SHOGLE_MATH_DEF T tan(T x) noexcept {
  return std::tan(x);
}

template<numeric_type T>
SHOGLE_MATH_DEF T acos(T x) noexcept {
  return std::acos(x);
}

template<numeric_type T>
SHOGLE_MATH_DEF T asin(T x) noexcept {
  return std::asin(x);
}

template<numeric_type T>
SHOGLE_MATH_DEF T atan(T x) noexcept {
  return std::atan(x);
}

template<numeric_type T>
SHOGLE_MATH_DEF T atan2(T y, T x) noexcept {
  return std::atan2(y, x);
}

template<numeric_type T>
SHOGLE_MATH_DEF T rad(T degs) noexcept {
  return degs * static_cast<T>(0.01745329251994329576923690768489);
}

template<typename T>
SHOGLE_MATH_DEF T deg(T rads) {
  return rads * static_cast<T>(57.295779513082320876798154814105);
}

template<numeric_type T>
SHOGLE_MATH_DEF T abs(T x) noexcept {
  return std::abs(x);
}

template<numeric_type T>
SHOGLE_MATH_DEF T clamp(T x, T min, T max) noexcept {
  return std::clamp(x, min, max);
}

template<numeric_type T>
SHOGLE_MATH_DEF T max(T a, T b) noexcept {
  return std::max(a, b);
}

template<numeric_type T>
SHOGLE_MATH_DEF T min(T a, T b) noexcept {
  return std::min(a, b);
}

template<numeric_type T>
SHOGLE_MATH_DEF T epsilon_err(T a, T b) noexcept {
  return ::shogle::math::abs((b - a) / b);
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

} // namespace shogle::math
