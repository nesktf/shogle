#define SHOGLE_MATH_VECTOR3_INL
#include "./vector3.hpp"
#undef SHOGLE_MATH_VECTOR3_INL

namespace shogle {

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numvec<3, T>& numvec<3, T>::operator=(U scalar) noexcept {
  this->x = static_cast<T>(scalar);
  this->y = static_cast<T>(scalar);
  this->z = static_cast<T>(scalar);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numvec<3, T>& numvec<3, T>::operator+=(U scalar) noexcept {
  this->x += static_cast<T>(scalar);
  this->y += static_cast<T>(scalar);
  this->z += static_cast<T>(scalar);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numvec<3, T>& numvec<3, T>::operator-=(U scalar) noexcept {
  this->x -= static_cast<T>(scalar);
  this->y -= static_cast<T>(scalar);
  this->z -= static_cast<T>(scalar);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numvec<3, T>& numvec<3, T>::operator*=(U scalar) noexcept {
  this->x *= static_cast<T>(scalar);
  this->y *= static_cast<T>(scalar);
  this->z *= static_cast<T>(scalar);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numvec<3, T>& numvec<3, T>::operator/=(U scalar) noexcept {
  this->x /= static_cast<T>(scalar);
  this->y /= static_cast<T>(scalar);
  this->z /= static_cast<T>(scalar);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numvec<3, T>& numvec<3, T>::operator=(const numvec<3, U>& other) noexcept {
  this->x = static_cast<T>(other.x);
  this->y = static_cast<T>(other.y);
  this->z = static_cast<T>(other.z);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numvec<3, T>& numvec<3, T>::operator+=(const numvec<3, U>& other) noexcept {
  this->x += static_cast<T>(other.x);
  this->y += static_cast<T>(other.y);
  this->z += static_cast<T>(other.z);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numvec<3, T>& numvec<3, T>::operator-=(const numvec<3, U>& other) noexcept {
  this->x -= static_cast<T>(other.x);
  this->y -= static_cast<T>(other.y);
  this->z -= static_cast<T>(other.z);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numvec<3, T>& numvec<3, T>::operator*=(const numvec<3, U>& other) noexcept {
  this->x *= static_cast<T>(other.x);
  this->y *= static_cast<T>(other.y);
  this->z *= static_cast<T>(other.z);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numvec<3, T>& numvec<3, T>::operator/=(const numvec<3, U>& other) noexcept {
  this->x /= static_cast<T>(other.x);
  this->y /= static_cast<T>(other.y);
  this->z /= static_cast<T>(other.z);
  return *this;
}

template<typename T>
SHOGLE_MATH_DEF bool operator==(const numvec<3, T>& a, const numvec<3, T>& b) noexcept {
  return (a.x == b.x) && (a.y == b.y) && (a.z == b.z);
}

template<typename T>
SHOGLE_MATH_DEF bool operator!=(const numvec<3, T>& a, const numvec<3, T>& b) noexcept {
  return (a.x != b.x) || (a.y != b.y) || (a.z != b.z);
}

template<typename T>
SHOGLE_MATH_DEF numvec<3, T> operator+(const numvec<3, T>& vec) noexcept {
  numvec<3, T> out;
  out.x = +vec.x;
  out.y = +vec.y;
  out.z = +vec.z;
  return out;
}

template<typename T>
SHOGLE_MATH_DEF numvec<3, T> operator+(const numvec<3, T>& a, const numvec<3, T>& b) noexcept {
  numvec<3, T> out;
  out.x = a.x + b.x;
  out.y = a.y + b.y;
  out.z = a.z + b.z;
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numvec<3, T> operator+(const numvec<3, T>& vec, U scalar) noexcept {
  numvec<3, T> out;
  out.x = vec.x + static_cast<T>(scalar);
  out.y = vec.y + static_cast<T>(scalar);
  out.z = vec.z + static_cast<T>(scalar);
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numvec<3, T> operator+(U scalar, const numvec<3, T>& vec) noexcept {
  numvec<3, T> out;
  out.x = static_cast<T>(scalar) + vec.x;
  out.y = static_cast<T>(scalar) + vec.y;
  out.z = static_cast<T>(scalar) + vec.z;
  return out;
}

template<typename T>
SHOGLE_MATH_DEF numvec<3, T> operator-(const numvec<3, T>& vec) noexcept {
  numvec<3, T> out;
  out.x = -vec.x;
  out.y = -vec.y;
  out.z = -vec.z;
  return out;
}

template<typename T>
SHOGLE_MATH_DEF numvec<3, T> operator-(const numvec<3, T>& a, const numvec<3, T>& b) noexcept {
  numvec<3, T> out;
  out.x = a.x - b.x;
  out.y = a.y - b.y;
  out.z = a.z - b.z;
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numvec<3, T> operator-(const numvec<3, T>& vec, U scalar) noexcept {
  numvec<3, T> out;
  out.x = vec.x - static_cast<T>(scalar);
  out.y = vec.y - static_cast<T>(scalar);
  out.z = vec.z - static_cast<T>(scalar);
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numvec<3, T> operator-(U scalar, const numvec<3, T>& vec) noexcept {
  numvec<3, T> out;
  out.x = static_cast<T>(scalar) - vec.x;
  out.y = static_cast<T>(scalar) - vec.y;
  out.z = static_cast<T>(scalar) - vec.z;
  return out;
}

template<typename T>
SHOGLE_MATH_DEF numvec<3, T> operator*(const numvec<3, T>& a, const numvec<3, T>& b) noexcept {
  numvec<3, T> out;
  out.x = a.x * b.x;
  out.y = a.y * b.y;
  out.z = a.z * b.z;
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numvec<3, T> operator*(const numvec<3, T>& vec, U scalar) noexcept {
  numvec<3, T> out;
  out.x = vec.x * static_cast<T>(scalar);
  out.y = vec.y * static_cast<T>(scalar);
  out.z = vec.z * static_cast<T>(scalar);
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numvec<3, T> operator*(U scalar, const numvec<3, T>& vec) noexcept {
  numvec<3, T> out = vec;
  out.x = static_cast<T>(scalar) * vec.x;
  out.y = static_cast<T>(scalar) * vec.y;
  out.z = static_cast<T>(scalar) * vec.z;
  return out;
}

template<typename T>
SHOGLE_MATH_DEF numvec<3, T> operator/(const numvec<3, T>& a, const numvec<3, T>& b) noexcept {
  numvec<3, T> out;
  out.x = a.x / b.x;
  out.y = a.y / b.y;
  out.z = a.z / b.z;
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numvec<3, T> operator/(const numvec<3, T>& vec, U scalar) noexcept {
  numvec<3, T> out;
  out.x = vec.x / static_cast<T>(scalar);
  out.y = vec.y / static_cast<T>(scalar);
  out.z = vec.z / static_cast<T>(scalar);
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numvec<3, T> operator/(U scalar, const numvec<3, T>& vec) noexcept {
  numvec<3, T> out = vec;
  out.x = static_cast<T>(scalar) / vec.x;
  out.y = static_cast<T>(scalar) / vec.y;
  out.z = static_cast<T>(scalar) / vec.z;
  return out;
}

template<typename U, typename T>
requires(math::numeric_convertible<U, T>)
SHOGLE_MATH_DEF numvec<3, U> vec_cast(const numvec<3, T>& vec) noexcept {
  return {static_cast<U>(vec.x), static_cast<U>(vec.y), static_cast<U>(vec.z)};
}

} // namespace shogle

namespace shogle::math {

template<typename T>
SHOGLE_MATH_DEF T length2(const numvec<3, T>& vec) noexcept {
  return (vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z);
}

template<typename T>
SHOGLE_MATH_DEF T length(const numvec<3, T>& vec) noexcept {
  return std::sqrt((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
}

template<typename T>
SHOGLE_MATH_DEF void normalize_at(numvec<3, T>& vec) noexcept {
  const T len = ::shogle::math::length(vec);
  vec.x /= len;
  vec.y /= len;
  vec.z /= len;
}

template<typename T>
SHOGLE_MATH_DEF numvec<3, T> normalize(const numvec<3, T>& vec) noexcept {
  numvec<3, T> out;
  const T len = ::shogle::math::length(vec);
  out.x = vec.x / len;
  out.y = vec.y / len;
  out.z = vec.z / len;
  return out;
}

template<typename T>
SHOGLE_MATH_DEF T dot(const numvec<3, T>& a, const numvec<3, T>& b) noexcept {
  return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

template<typename T>
SHOGLE_MATH_DEF numvec<3, T> cross(const numvec<3, T>& a, const numvec<3, T>& b) noexcept {
  numvec<3, T> out;
  out.x = (a.y * b.z) - (b.y * a.z);
  out.y = (a.z * b.x) - (b.z * a.x);
  out.z = (a.x * b.y) - (b.x * a.y);
  return out;
}

template<numeric_type T>
SHOGLE_MATH_DEF void gl_to_cartesian_at(numvec<3, T>& vec) noexcept {
  const T x = vec.x;
  vec.x = vec.z;
  vec.z = vec.y;
  vec.y = x;
}

template<typename T>
SHOGLE_MATH_DEF numvec<3, T> gl_to_cartesian(const numvec<3, T>& vec) noexcept {
  numvec<3, T> out;
  out.x = vec.z;
  out.y = vec.x;
  out.z = vec.y;
  return out;
}

template<numeric_type T>
SHOGLE_MATH_DEF numvec<3, T> sph_to_cartesian(T rho, T theta, T phi) noexcept {
  numvec<3, T> out;
  out.x = rho * std::sin(theta) * std::cos(phi);
  out.y = rho * std::sin(theta) * std::sin(phi);
  out.z = rho * std::cos(theta);
  return out;
}

template<numeric_type T>
SHOGLE_MATH_DEF void cartesian_to_gl_at(numvec<3, T>& vec) noexcept {
  const T x = vec.x;
  vec.x = vec.y;
  vec.y = vec.z;
  vec.z = x;
}

template<typename T>
SHOGLE_MATH_DEF numvec<3, T> cartesian_to_gl(const numvec<3, T>& vec) noexcept {
  numvec<3, T> out;
  out.x = vec.y;
  out.y = vec.z;
  out.z = vec.x;
  return out;
}

} // namespace shogle::math
