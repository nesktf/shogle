#pragma once

#include "./vector2.hpp"
#include "./vector3.hpp"

namespace shogle {

template<math::numeric_type T>
struct nummat<3, 3, T> {
public:
  using value_type = T;
  static constexpr u32 component_count = 9;

  using row_type = numvec<3, T>;
  using col_type = numvec<3, T>;

public:
  SHOGLE_MATH_DECLARE_VECTOR_SPECIAL_MEMBERS(nummat);

  SHOGLE_MATH_DEF explicit nummat(T scalar) noexcept :
      x1(scalar), y1(), z1(), x2(), y2(scalar), z2(), x3(), y3(), z3(scalar) {}

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DEF explicit nummat(U scalar) noexcept :
      x1(static_cast<T>(scalar)), y1(), z1(), x2(), y2(static_cast<T>(scalar)), z2(), x3(), y3(),
      z3(static_cast<T>(scalar)) {}

  SHOGLE_MATH_DEF nummat(T x1_, T y1_, T z1_, T x2_, T y2_, T z2_, T x3_, T y3_, T z3_) noexcept :
      x1(x1_), y1(y1_), z1(z1_), x2(x2_), y2(y2_), z2(z2_), x3(x3_), y3(y3_), z3(z3_) {}

  template<typename X1, typename Y1, typename Z1, typename X2, typename Y2, typename Z2,
           typename X3, typename Y3, typename Z3>
  SHOGLE_MATH_DEF nummat(X1 x1_, Y1 y1_, Z1 z1_, X2 x2_, Y2 y2_, Z2 z2_, X3 x3_, Y3 y3_,
                         Z3 z3_) noexcept :
      x1(static_cast<T>(x1_)), y1(static_cast<T>(y1_)), z1(static_cast<T>(z1_)),
      x2(static_cast<T>(x2_)), y2(static_cast<T>(y2_)), z2(static_cast<T>(z2_)),
      x3(static_cast<T>(x3_)), y3(static_cast<T>(y3_)), z3(static_cast<T>(z3_)) {}

  template<typename V1, typename V2, typename V3>
  SHOGLE_MATH_DEF nummat(const numvec<3, V1>& v1, const numvec<3, V2>& v2,
                         const numvec<3, V3>& v3) noexcept :
      x1(static_cast<T>(v1.x)), y1(static_cast<T>(v1.y)), z1(static_cast<T>(v1.z)),
      x2(static_cast<T>(v2.x)), y2(static_cast<T>(v2.y)), z2(static_cast<T>(v2.z)),
      x3(static_cast<T>(v3.x)), y3(static_cast<T>(v3.y)), z3(static_cast<T>(v3.z)) {}

public:
  SHOGLE_MATH_DEF const T& operator[](size_t idx) const noexcept {
    switch (idx) {
      default:
        [[fallthrough]];
      case 0:
        return x1;
      case 1:
        return y1;
      case 2:
        return z1;
      case 3:
        return x2;
      case 4:
        return y2;
      case 5:
        return z2;
      case 6:
        return x3;
      case 7:
        return y3;
      case 8:
        return z3;
    }
  }

  SHOGLE_MATH_DEF T& operator[](size_t idx) noexcept {
    return const_cast<T&>(std::as_const(*this)[idx]);
  }

  SHOGLE_MATH_DEF col_type column_at(size_t idx) const noexcept {
    switch (idx) {
      default:
        [[fallthrough]];
      case 0:
        return {x1, y1, z1};
      case 1:
        return {x2, y2, z2};
      case 2:
        return {x3, y3, z3};
    }
  }

  SHOGLE_MATH_DEF row_type row_at(size_t idx) const noexcept {
    switch (idx) {
      default:
        [[fallthrough]];
      case 0:
        return {x1, x2, x3};
      case 1:
        return {y1, y2, y3};
      case 2:
        return {z1, z2, z3};
    }
  }

public:
  SHOGLE_MATH_DEF const T* data() const noexcept { return &this->x1; }

  SHOGLE_MATH_DEF T* data() noexcept { return const_cast<T*>(std::as_const(*this).data()); }

public:
  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL nummat& operator=(U scalar) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL nummat& operator+=(U scalar) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL nummat& operator-=(U scalar) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL nummat& operator*=(U scalar) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL nummat& operator/=(U scalar) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL nummat& operator=(const nummat<3, 3, U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL nummat& operator+=(const nummat<3, 3, U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL nummat& operator-=(const nummat<3, 3, U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL nummat& operator*=(const nummat<3, 3, U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL nummat& operator/=(const nummat<3, 3, U>& other) noexcept;

public:
  // Column major ordering
  T x1, y1, z1;
  T x2, y2, z2;
  T x3, y3, z3;
};

template<typename T>
SHOGLE_MATH_DECL bool operator==(const nummat<3, 3, T>& a, const nummat<3, 3, T>& b) noexcept;

template<typename T>
SHOGLE_MATH_DECL bool operator!=(const nummat<3, 3, T>& a, const nummat<3, 3, T>& b) noexcept;

template<typename T>
SHOGLE_MATH_DECL nummat<3, 3, T> operator+(const nummat<3, 3, T>& mat) noexcept;

template<typename T>
SHOGLE_MATH_DECL nummat<3, 3, T> operator+(const nummat<3, 3, T>& a,
                                           const nummat<3, 3, T>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<3, 3, T> operator+(const nummat<3, 3, T>& mat, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<3, 3, T> operator+(U scalar, const nummat<3, 3, T>& mat) noexcept;

template<typename T>
SHOGLE_MATH_DECL nummat<3, 3, T> operator-(const nummat<3, 3, T>& mat) noexcept;

template<typename T>
SHOGLE_MATH_DECL nummat<3, 3, T> operator-(const nummat<3, 3, T>& a,
                                           const nummat<3, 3, T>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<3, 3, T> operator-(const nummat<3, 3, T>& mat, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<3, 3, T> operator-(U scalar, const nummat<3, 3, T>& mat) noexcept;

template<typename T>
SHOGLE_MATH_DECL nummat<3, 3, T> operator*(const nummat<3, 3, T>& a,
                                           const nummat<3, 3, T>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<3, 3, T> operator*(const nummat<3, 3, T>& mat, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<3, 3, T> operator*(U scalar, const nummat<3, 3, T>& mat) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL typename nummat<3, 3, T>::col_type operator*(const nummat<3, 3, T>& mat,
                                                              const numvec<3, U>& vec) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL typename nummat<3, 3, T>::row_type operator*(const numvec<3, U>& vec,
                                                              const nummat<3, 3, T>& mat) noexcept;

template<typename T>
SHOGLE_MATH_DECL nummat<3, 3, T> operator/(const nummat<3, 3, T>& a,
                                           const nummat<3, 3, T>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<3, 3, T> operator/(const nummat<3, 3, T>& mat, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<3, 3, T> operator/(U scalar, const nummat<3, 3, T>& mat) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL typename nummat<3, 3, T>::col_type operator/(const nummat<3, 3, T>& mat,
                                                              const numvec<3, U>& vec) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL typename nummat<3, 3, T>::row_type operator/(const numvec<3, U>& vec,
                                                              const nummat<3, 3, T>& mat) noexcept;

} // namespace shogle

namespace shogle::math {

template<typename T>
SHOGLE_MATH_DECL T determinant(const nummat<3, 3, T>& m) noexcept;

template<typename T>
SHOGLE_MATH_DECL nummat<3, 3, T> inverse(const nummat<3, 3, T>& m) noexcept;

template<typename T>
SHOGLE_MATH_DECL nummat<3, 3, T> transpose(const nummat<3, 3, T>& m) noexcept;

template<typename T, numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<3, 3, T> translate(const nummat<3, 3, T>& m,
                                           const numvec<2, U>& v) noexcept;

template<typename T, numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<3, 3, T> scale(const nummat<3, 3, T>& m, const numvec<2, U>& v) noexcept;

template<typename T, numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<3, 3, T> scale(const nummat<3, 3, T>& m, const numvec<3, U>& v) noexcept;

template<typename T, numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<3, 3, T> scale(const nummat<3, 3, T>& m, U angle) noexcept;

template<typename T, numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<3, 3, T> rotate(const nummat<3, 3, T>& m, U angle) noexcept;

} // namespace shogle::math

#ifndef SHOGLE_MATH_MATRIX3X3_INL
#include "./matrix3x3.inl"
#endif
