#pragma once

#include <shogle/math/common.hpp>

namespace shogle {

template<math::numeric_type T>
struct numvec<4, T> {
public:
  using value_type = T;
  static constexpr u32 component_count = 4;

public:
  SHOGLE_MATH_DECLARE_VECTOR_SPECIAL_MEMBERS(numvec);

  SHOGLE_MATH_DEF numvec(T x_, T y_, T z_, T w_) noexcept : x(x_), y(y_), z(z_), w(w_) {}

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DEF explicit numvec(U scalar) noexcept :
      x(static_cast<T>(scalar)), y(static_cast<T>(scalar)), z(static_cast<T>(scalar)),
      w(static_cast<T>(scalar)) {}

  template<math::numeric_convertible<T> X, math::numeric_convertible<T> Y,
           math::numeric_convertible<T> Z, math::numeric_convertible<T> W>
  SHOGLE_MATH_DEF numvec(X x_, Y y_, Z z_, W w_) noexcept :
      x(static_cast<T>(x_)), y(static_cast<T>(y_)), z(static_cast<T>(z_)), w(static_cast<T>(w_)) {}

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
      case 3:
        return w;
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
  SHOGLE_MATH_DECL numvec& operator=(const numvec<4, U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numvec& operator+=(const numvec<4, U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numvec& operator-=(const numvec<4, U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numvec& operator*=(const numvec<4, U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numvec& operator/=(const numvec<4, U>& other) noexcept;

public:
  T x, y, z, w;
};

template<typename T>
SHOGLE_MATH_DECL bool operator==(const numvec<4, T>& a, const numvec<4, T>& b) noexcept;

template<typename T>
SHOGLE_MATH_DECL bool operator!=(const numvec<4, T>& a, const numvec<4, T>& b) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<4, T> operator+(const numvec<4, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<4, T> operator+(const numvec<4, T>& a, const numvec<4, T>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<4, T> operator+(const numvec<4, T>& vec, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<4, T> operator+(U scalar, const numvec<4, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<4, T> operator-(const numvec<4, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<4, T> operator-(const numvec<4, T>& a, const numvec<4, T>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<4, T> operator-(const numvec<4, T>& vec, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<4, T> operator-(U scalar, const numvec<4, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<4, T> operator*(const numvec<4, T>& a, const numvec<4, T>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<4, T> operator*(const numvec<4, T>& vec, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<4, T> operator*(U scalar, const numvec<4, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<4, T> operator/(const numvec<4, T>& a, const numvec<4, T>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<4, T> operator/(const numvec<4, T>& vec, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<4, T> operator/(U scalar, const numvec<4, T>& vec) noexcept;

} // namespace shogle

namespace shogle::math {

template<typename T>
SHOGLE_MATH_DECL T length2(const numvec<4, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL T length(const numvec<4, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL void normalize_at(numvec<4, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<4, T> normalize(const numvec<4, T>& vec) noexcept;

template<typename T>
SHOGLE_MATH_DECL T dot(const numvec<4, T>& a, const numvec<4, T>& b) noexcept;

} // namespace shogle::math

#ifndef SHOGLE_MATH_VECTOR4_INL
#include "./vector4.inl"
#endif
