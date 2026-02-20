#pragma once

#include "./vector3.hpp"
#include "./vector4.hpp"

#include "./matrix3x3.hpp"
#include "./matrix4x4.hpp"

namespace shogle {

template<math::numeric_type T>
struct numquat {
public:
  using value_type = T;
  static constexpr u32 component_count = 4;

  SHOGLE_SELF_STATIC_CONST_MEMBER numquat identity = T(1);

public:
  SHOGLE_MATH_DECLARE_VECTOR_SPECIAL_MEMBERS(numquat);

  SHOGLE_MATH_DEF numquat(T w_, T x_, T y_, T z_) noexcept : w(w_), x(x_), y(y_), z(z_) {}

  template<math::numeric_convertible<T> W, math::numeric_convertible<T> X,
           math::numeric_convertible<T> Y, math::numeric_convertible<T> Z>
  SHOGLE_MATH_DEF numquat(W w_, X x_, Y y_, Z z_) noexcept :
      w(static_cast<T>(w_)), x(static_cast<T>(x_)), y(static_cast<T>(y_)), z(static_cast<T>(z_)) {}

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DEF explicit numquat(const numquat<U>& other) noexcept :
      w(static_cast<T>(other.w)), x(static_cast<T>(other.x)), y(static_cast<T>(other.y)),
      z(static_cast<T>(other.z)) {}

  template<math::numeric_convertible<T> U, math::numeric_convertible<T> V>
  SHOGLE_MATH_DEF numquat(U w_, const numvec<3, V>& v) noexcept :
      w(static_cast<T>(w_)), x(static_cast<T>(v.x)), y(static_cast<T>(v.y)),
      z(static_cast<T>(v.z)) {}

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL explicit numquat(const numvec<3, U>& euler) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL explicit numquat(const nummat<3, 3, U>& m) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL explicit numquat(const nummat<4, 4, U>& m) noexcept;

public:
  SHOGLE_MATH_DEF const T& operator[](size_t idx) const noexcept {
    switch (idx) {
      default:
        [[fallthrough]];
      case 0:
        return w;
      case 1:
        return x;
      case 2:
        return y;
      case 3:
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
  SHOGLE_MATH_DECL numquat& operator=(U scalar) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numquat& operator+=(U scalar) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numquat& operator-=(U scalar) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numquat& operator*=(U scalar) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numquat& operator/=(U scalar) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numquat& operator=(const numquat<U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numquat& operator+=(const numquat<U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numquat& operator-=(const numquat<U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL numquat& operator*=(const numquat<U>& other) noexcept;

public:
  T w, x, y, z;
};

template<typename T>
SHOGLE_MATH_DECL bool operator==(const numquat<T>& a, const numquat<T>& b) noexcept;

template<typename T>
SHOGLE_MATH_DECL bool operator!=(const numquat<T>& a, const numquat<T>& b) noexcept;

template<typename T>
SHOGLE_MATH_DECL numquat<T> operator+(const numquat<T>& q) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numquat<T> operator+(const numquat<T>& a, const numquat<U>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numquat<T> operator+(const numquat<T>& q, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numquat<T> operator+(U scalar, const numquat<T>& q) noexcept;

template<typename T>
SHOGLE_MATH_DECL numquat<T> operator-(const numquat<T>& q) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numquat<T> operator-(const numquat<T>& a, const numquat<U>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numquat<T> operator-(const numquat<T>& q, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numquat<T> operator-(U scalar, const numquat<T>& q) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numquat<T> operator*(const numquat<T>& a, const numquat<U>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<3, T> operator*(const numquat<T>& q, const numvec<3, U>& v) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<3, T> operator*(const numvec<3, U>& v, const numquat<T>& q) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<4, T> operator*(const numquat<T>& q, const numvec<4, U>& v) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numvec<4, T> operator*(const numvec<4, U>& v, const numquat<T>& q) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numquat<T> operator*(const numquat<T>& q, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numquat<T> operator*(U scalar, const numquat<T>& q) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numquat<T> operator/(const numquat<T>& q, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numquat<T> operator/(U scalar, const numquat<T>& q) noexcept;

template<typename U, typename T>
requires(math::numeric_convertible<U, T>)
SHOGLE_MATH_DECL numquat<U> vec_cast(const numquat<T>& q) noexcept;

} // namespace shogle

namespace shogle::math {

template<typename T>
SHOGLE_MATH_DECL numquat<T> conjugate(const numquat<T>& q) noexcept;

template<typename T>
SHOGLE_MATH_DECL numquat<T> inverse(const numquat<T>& q) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL T dot(const numquat<T>& a, const numquat<U>& b) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<3, T> to_euler(const numquat<T>& q) noexcept;

template<typename T>
SHOGLE_MATH_DECL T roll(const numquat<T>& q) noexcept;

template<typename T>
SHOGLE_MATH_DECL T pitch(const numquat<T>& q) noexcept;

template<typename T>
SHOGLE_MATH_DECL T yaw(const numquat<T>& q) noexcept;

template<typename T>
SHOGLE_MATH_DECL nummat<3, 3, T> to_mat3(const numquat<T>& q) noexcept;

template<typename T>
SHOGLE_MATH_DECL nummat<4, 4, T> to_mat4(const numquat<T>& q) noexcept;

template<typename T>
SHOGLE_MATH_DECL numquat<T> to_quat(const nummat<3, 3, T>& m) noexcept;

template<typename T>
SHOGLE_MATH_DECL numquat<T> to_quat(const nummat<4, 4, T>& m) noexcept;

template<typename T>
SHOGLE_MATH_DECL numquat<T> to_quat(const numvec<3, T>& euler) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL numquat<T> to_quat(T angle, const numvec<3, U>& axis) noexcept;

template<typename T>
SHOGLE_MATH_DECL numvec<3, T> to_vec3(const numquat<T>& q) noexcept;

} // namespace shogle::math

#ifndef SHOGLE_MATH_QUATERNION_INL
#include "./quaternion.inl"
#endif
