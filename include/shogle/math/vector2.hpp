#pragma once

#include <shogle/math/common.hpp>

namespace shogle {

template<math::numeric_type T>
struct numvec<2, T> {
public:
  using value_type = T;
  static constexpr u32 component_count = 2;

public:
  SHOGLE_MATH_DECLARE_VECTOR_SPECIAL_MEMBERS(numvec);

  SHOGLE_MATH_DEF numvec(T x_, T y_) noexcept : x(x_), y(y_) {}

  SHOGLE_MATH_DEF explicit numvec(T scalar) noexcept : x(scalar), y(scalar) {}

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL explicit numvec(U scalar) noexcept :
      x(static_cast<T>(scalar)), y(static_cast<T>(scalar)) {}

  template<math::numeric_type X, math::numeric_type Y>
  SHOGLE_MATH_DECL numvec(X x_, Y y_) noexcept : x(static_cast<T>(x_)), y(static_cast<T>(y_)) {}

public:
  SHOGLE_MATH_DEF const T& operator[](size_t idx) const noexcept {
    switch (idx) {
      default:
        [[fallthrough]];
      case 0:
        return x;
      case 1:
        return y;
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
  SHOGLE_MATH_DECL numvec& operator=(const numvec<2, U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numvec& operator+=(const numvec<2, U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numvec& operator-=(const numvec<2, U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numvec& operator*=(const numvec<2, U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numvec& operator/=(const numvec<2, U>& other) noexcept;

public:
  T x, y;
};

template<typename T>
SHOGLE_MATH_DECL numvec<2, T> operator==(const numvec<2, T>& a, const numvec<2, T>& b) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<2, T> operator!=(const numvec<2, T>& a, const numvec<2, T>& b) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<2, T> operator+(const numvec<2, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<2, T> operator+(const numvec<2, T>& a, const numvec<2, T>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<2, T> operator+(const numvec<2, T>& vec, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<2, T> operator+(U scalar, const numvec<2, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<2, T> operator-(const numvec<2, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<2, T> operator-(const numvec<2, T>& a, const numvec<2, T>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<2, T> operator-(const numvec<2, T>& vec, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<2, T> operator-(U scalar, const numvec<2, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<2, T> operator*(const numvec<2, T>& a, const numvec<2, T>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<2, T> operator*(const numvec<2, T>& vec, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<2, T> operator*(U scalar, const numvec<2, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<2, T> operator/(const numvec<2, T>& a, const numvec<2, T>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<2, T> operator/(const numvec<2, T>& vec, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<2, T> operator/(U scalar, const numvec<2, T>& vec) noexcept;

} // namespace shogle

namespace shogle::math {

template<typename T>
SHOGLE_MATH_DECL T length2(const numvec<2, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL T length(const numvec<2, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL void normalize_at(numvec<2, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<2, T> normalize(const numvec<2, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL T dot(const numvec<2, T>& a, const numvec<2, T>& b) noexcept;

} // namespace shogle::math

#ifndef SHOGLE_MATH_VECTOR2_INL
#include "./vector2.inl"
#endif
