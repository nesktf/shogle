#pragma once

#include <shogle/math/common.hpp>

namespace shogle {

template<math::numeric_type T>
struct numvec<3, T> {
public:
  using value_type = T;
  static constexpr u32 component_count = 3;

public:
  SHOGLE_MATH_DECLARE_VECTOR_SPECIAL_MEMBERS(numvec);

  SHOGLE_MATH_DEF numvec(T x_, T y_, T z_) noexcept : x(x_), y(y_), z(z_) {}

  SHOGLE_MATH_DEF explicit numvec(T scalar) noexcept : x(scalar), y(scalar), z(scalar) {}

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DEF explicit numvec(U scalar) noexcept :
      x(static_cast<T>(scalar)), y(static_cast<T>(scalar)), z(static_cast<T>(scalar)) {}

  template<math::numeric_convertible<T> X, math::numeric_convertible<T> Y,
           math::numeric_convertible<T> Z>
  SHOGLE_MATH_DEF numvec(X x_, Y y_, Z z_) noexcept :
      x(static_cast<T>(x_)), y(static_cast<T>(y_)), z(static_cast<T>(z_)) {}

public:
  SHOGLE_MATH_DEF const T& operator[](size_t idx) const noexcept {
    switch (idx) {
      default:
        [[fallthrough]];
      case 0:
        return x;
      case 1:
        return y;
      case 2:
        return z;
    }
  }

  SHOGLE_MATH_DEF T& operator[](size_t idx) noexcept {
    return const_cast<T&>(std::as_const(*this)[idx]);
  }

public:
  SHOGLE_MATH_DEF const T* data() const noexcept { return &this->x; }

  SHOGLE_MATH_DEF T* data() noexcept { return const_cast<T*>(std::as_const(*this).data()); }

public:
  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numvec& operator=(U scalar) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numvec& operator+=(U scalar) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numvec& operator-=(U scalar) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numvec& operator*=(U scalar) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numvec& operator/=(U scalar) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numvec& operator=(const numvec<3, U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numvec& operator+=(const numvec<3, U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numvec& operator-=(const numvec<3, U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numvec& operator*=(const numvec<3, U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numvec& operator/=(const numvec<3, U>& other) noexcept;

public:
  T x, y, z;
};

template<typename T>
SHOGLE_MATH_DECL bool operator==(const numvec<3, T>& a, const numvec<3, T>& b) noexcept;

template<typename T>
SHOGLE_MATH_DECL bool operator!=(const numvec<3, T>& a, const numvec<3, T>& b) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<3, T> operator+(const numvec<3, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<3, T> operator+(const numvec<3, T>& a, const numvec<3, T>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<3, T> operator+(const numvec<3, T>& vec, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<3, T> operator+(U scalar, const numvec<3, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<3, T> operator-(const numvec<3, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<3, T> operator-(const numvec<3, T>& a, const numvec<3, T>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<3, T> operator-(const numvec<3, T>& vec, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<3, T> operator-(U scalar, const numvec<3, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<3, T> operator*(const numvec<3, T>& a, const numvec<3, T>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<3, T> operator*(const numvec<3, T>& vec, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<3, T> operator*(U scalar, const numvec<3, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<3, T> operator/(const numvec<3, T>& a, const numvec<3, T>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<3, T> operator/(const numvec<3, T>& vec, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<3, T> operator/(U scalar, const numvec<3, T>& vec) noexcept;

template<typename U, typename T>
requires(math::numeric_convertible<U, T>)
SHOGLE_MATH_DECL numvec<3, U> vec_cast(const numvec<3, T>& vec) noexcept;

} // namespace shogle

namespace shogle::math {

template<typename T>
SHOGLE_MATH_DECL T length2(const numvec<3, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL T length(const numvec<3, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL void normalize_at(numvec<3, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<3, T> normalize(const numvec<3, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL T dot(const numvec<3, T>& a, const numvec<3, T>& b) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<3, T> cross(const numvec<3, T>& a, const numvec<3, T>& b) noexcept;

template<numeric_type T>
SHOGLE_MATH_DECL void gl_to_cartesian_at(numvec<3, T>& vec) noexcept;

template<numeric_type T>
SHOGLE_MATH_DECL numvec<3, T> gl_to_cartesian(const numvec<3, T>& vec) noexcept;

template<numeric_type T>
SHOGLE_MATH_DECL numvec<3, T> sph_to_cartesian(T rho, T theta, T phi) noexcept;

template<numeric_type T>
SHOGLE_MATH_DECL void cartesian_to_gl_at(numvec<3, T>& vec) noexcept;

template<numeric_type T>
SHOGLE_MATH_DECL numvec<3, T> cartesian_to_gl(const numvec<3, T>& vec) noexcept;

} // namespace shogle::math

#ifndef SHOGLE_MATH_VECTOR3_INL
#include "./vector3.inl"
#endif
