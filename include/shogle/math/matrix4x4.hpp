#pragma once

#include "./vector4.hpp"

namespace shogle {

template<math::numeric_type T>
struct nummat<4, 4, T> {
public:
  using value_type = T;
  static constexpr u32 component_count = 9;

  using row_type = numvec<4, T>;
  using col_type = numvec<4, T>;

public:
  SHOGLE_MATH_DECLARE_VECTOR_SPECIAL_MEMBERS(nummat);

  SHOGLE_MATH_DEF explicit nummat(T scalar) noexcept :
      x1(scalar), y1(), z1(), w1(), x2(), y2(scalar), z2(), w2(), x3(), y3(), z3(scalar), w3(),
      x4(), y4(), z4(), w4(scalar) {}

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DEF explicit nummat(U scalar) noexcept :
      x1(static_cast<T>(scalar)), y1(), z1(), w1(), x2(), y2(static_cast<T>(scalar)), z2(), w2(),
      x3(), y3(), z3(static_cast<T>(scalar)), w3(), x4(), y4(), z4(), w4(static_cast<T>(scalar)) {}

  SHOGLE_MATH_DEF nummat(T x1_, T y1_, T z1_, T w1_, T x2_, T y2_, T z2_, T w2_, T x3_, T y3_,
                         T z3_, T w3_, T x4_, T y4_, T z4_, T w4_) noexcept :
      x1(x1_), y1(y1_), z1(z1_), w1(w1_), x2(x2_), y2(y2_), z2(z2_), w2(w2_), x3(x3_), y3(y3_),
      z3(z3_), w3(w3_), x4(x4_), y4(y4_), z4(z4_), w4(w4_) {}

  template<typename X1, typename Y1, typename Z1, typename W1, typename X2, typename Y2,
           typename Z2, typename W2, typename X3, typename Y3, typename Z3, typename W3,
           typename X4, typename Y4, typename Z4, typename W4>
  SHOGLE_MATH_DEF nummat(X1 x1_, Y1 y1_, Z1 z1_, W1 w1_, X2 x2_, Y2 y2_, Z2 z2_, W2 w2_, X3 x3_,
                         Y3 y3_, Z3 z3_, W3 w3_, X4 x4_, Y4 y4_, Z4 z4_, W4 w4_) noexcept :
      x1(static_cast<T>(x1_)), y1(static_cast<T>(y1_)), z1(static_cast<T>(z1_)),
      w1(static_cast<T>(w1_)), x2(static_cast<T>(x2_)), y2(static_cast<T>(y2_)),
      z2(static_cast<T>(z2_)), w2(static_cast<T>(w2_)), x3(static_cast<T>(x3_)),
      y3(static_cast<T>(y3_)), z3(static_cast<T>(z3_)), w3(static_cast<T>(w3_)),
      x4(static_cast<T>(x4_)), y4(static_cast<T>(y4_)), z4(static_cast<T>(z4_)),
      w4(static_cast<T>(w4_)) {}

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
        return {x1, y1, z1, w1};
      case 1:
        return {x2, y2, z2, w2};
      case 2:
        return {x3, y3, z3, w3};
      case 3:
        return {x4, y4, z4, w4};
    }
  }

  SHOGLE_MATH_DEF row_type row_at(size_t idx) const noexcept {
    switch (idx) {
      default:
        [[fallthrough]];
      case 0:
        return {x1, x2, x3, x4};
      case 1:
        return {y1, y2, y3, y4};
      case 2:
        return {z1, z2, z3, z4};
      case 3:
        return {w1, w2, w3, w4};
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
  SHOGLE_MATH_DECL nummat& operator=(const nummat<4, 4, U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL nummat& operator+=(const nummat<4, 4, U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL nummat& operator-=(const nummat<4, 4, U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL nummat& operator*=(const nummat<4, 4, U>& other) noexcept;

  template<math::numeric_convertible<T> U>
  SHOGLE_MATH_DECL nummat& operator/=(const nummat<4, 4, U>& other) noexcept;

public:
  // Column major ordering
  T x1, y1, z1, w1;
  T x2, y2, z2, w2;
  T x3, y3, z3, w3;
  T x4, y4, z4, w4;
};

template<typename T>
SHOGLE_MATH_DECL bool operator==(const nummat<4, 4, T>& a, const nummat<4, 4, T>& b) noexcept;

template<typename T>
SHOGLE_MATH_DECL bool operator!=(const nummat<4, 4, T>& a, const nummat<4, 4, T>& b) noexcept;

template<typename T>
SHOGLE_MATH_DECL nummat<4, 4, T> operator+(const nummat<4, 4, T>& mat) noexcept;

template<typename T>
SHOGLE_MATH_DECL nummat<4, 4, T> operator+(const nummat<4, 4, T>& a,
                                           const nummat<4, 4, T>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<4, 4, T> operator+(const nummat<4, 4, T>& mat, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<4, 4, T> operator+(U scalar, const nummat<4, 4, T>& mat) noexcept;

template<typename T>
SHOGLE_MATH_DECL nummat<4, 4, T> operator-(const nummat<4, 4, T>& mat) noexcept;

template<typename T>
SHOGLE_MATH_DECL nummat<4, 4, T> operator-(const nummat<4, 4, T>& a,
                                           const nummat<4, 4, T>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<4, 4, T> operator-(const nummat<4, 4, T>& mat, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<4, 4, T> operator-(U scalar, const nummat<4, 4, T>& mat) noexcept;

template<typename T>
SHOGLE_MATH_DECL nummat<4, 4, T> operator*(const nummat<4, 4, T>& a,
                                           const nummat<4, 4, T>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<4, 4, T> operator*(const nummat<4, 4, T>& mat, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<4, 4, T> operator*(U scalar, const nummat<4, 4, T>& mat) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL typename nummat<4, 4, T>::col_type operator*(const nummat<4, 4, T>& mat,
                                                              const numvec<4, U>& vec) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL typename nummat<4, 4, T>::row_type operator*(const numvec<4, U>& vec,
                                                              const nummat<4, 4, T>& mat) noexcept;

template<typename T>
SHOGLE_MATH_DECL nummat<4, 4, T> operator/(const nummat<4, 4, T>& a,
                                           const nummat<4, 4, T>& b) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<4, 4, T> operator/(const nummat<4, 4, T>& mat, U scalar) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<4, 4, T> operator/(U scalar, const nummat<4, 4, T>& mat) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL typename nummat<4, 4, T>::col_type operator/(const nummat<4, 4, T>& mat,
                                                              const numvec<4, U>& vec) noexcept;

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DECL typename nummat<4, 4, T>::row_type operator/(const numvec<4, U>& vec,
                                                              const nummat<4, 4, T>& mat) noexcept;

} // namespace shogle

namespace shogle::math {

template<typename T>
SHOGLE_MATH_DECL T determinant(const nummat<4, 4, T>& m) noexcept;

template<typename T>
SHOGLE_MATH_DECL nummat<4, 4, T> inverse(const nummat<4, 4, T>& m) noexcept;

template<typename T>
SHOGLE_MATH_DECL nummat<4, 4, T> transpose(const nummat<4, 4, T>& m) noexcept;

template<typename T, numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<4, 4, T> translate(const nummat<4, 4, T>& m,
                                           const numvec<3, U>& v) noexcept;

} // namespace shogle::math

#ifndef SHOGLE_MATH_MATRIX4x4_INL
#include "./matrix4x4.inl"
#endif
