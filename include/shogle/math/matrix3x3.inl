#define SHOGLE_MATH_MATRIX3X3_INL
#include "./matrix3x3.hpp"
#undef SHOGLE_MATH_MATRIX3X3_INL

namespace shogle::math {

template<typename T>
SHOGLE_MATH_DEF T determinant(const nummat<3, 3, T>& m) noexcept {
  return ((m.x1 * m.y2 * m.z3) - (m.x1 * m.z2 * m.y3)) -
         ((m.x2 * m.y1 * m.z3) - (m.x2 * m.z1 * m.y3)) +
         ((m.x3 * m.y1 * m.z1) - (m.x3 * m.z1 * m.y2));
}

template<typename T>
SHOGLE_MATH_DEF nummat<3, 3, T> inverse(const nummat<3, 3, T>& m) noexcept {
  const T invdet = static_cast<T>(1) / ::shogle::math::determinant(m);
  nummat<3, 3, T> out;
  out.x1 = ((m.y2 * m.z3) - (m.z2 * m.y3)) * invdet;
  out.y1 = ((m.z1 * m.y3) - (m.y1 * m.z3)) * invdet;
  out.z1 = ((m.y1 * m.z2) - (m.z1 * m.y2)) * invdet;
  out.x2 = ((m.z2 * m.x3) - (m.x2 * m.z3)) * invdet;
  out.y2 = ((m.x1 * m.z3) - (m.z1 * m.x3)) * invdet;
  out.z2 = ((m.z1 * m.x2) - (m.x1 * m.z2)) * invdet;
  out.x3 = ((m.x2 * m.y3) - (m.y2 * m.x3)) * invdet;
  out.y3 = ((m.y1 * m.x3) - (m.x1 * m.y3)) * invdet;
  out.z3 = ((m.x1 * m.y2) - (m.y1 * m.x2)) * invdet;
  return out;
}

template<typename T>
SHOGLE_MATH_DEF nummat<3, 3, T> transpose(const nummat<3, 3, T>& m) noexcept {
  nummat<3, 3, T> out;
  out.x1 = m.x1;
  out.y1 = m.x2;
  out.z1 = m.x3;
  out.x2 = m.y1;
  out.y2 = m.y2;
  out.z2 = m.y3;
  out.x3 = m.z1;
  out.y3 = m.z2;
  out.z3 = m.z3;
  return out;
}

template<typename T, numeric_convertible<T> U>
SHOGLE_MATH_DECL nummat<3, 3, T> translate(const nummat<3, 3, T>& m,
                                           const numvec<2, U>& v) noexcept {
  nummat<3, 3, T> out;
  out.x3 = (m.x1 * static_cast<T>(v.x)) + (m.x2 * static_cast<T>(v.y)) + m.x3;
  out.y3 = (m.y1 * static_cast<T>(v.x)) + (m.y2 * static_cast<T>(v.y)) + m.y3;
  out.z3 = (m.z1 * static_cast<T>(v.x)) + (m.z2 * static_cast<T>(v.y)) + m.z3;
  return out;
}

template<typename T, numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<3, 3, T> scale(const nummat<3, 3, T>& m, const numvec<2, U>& v) noexcept {
  nummat<3, 3, T> out;
  out.x1 = m.x1 * static_cast<T>(v.x);
  out.y1 = m.y1 * static_cast<T>(v.x);
  out.z1 = m.z1 * static_cast<T>(v.x);
  out.x2 = m.x2 * static_cast<T>(v.y);
  out.y2 = m.y2 * static_cast<T>(v.y);
  out.z2 = m.z2 * static_cast<T>(v.y);
  out.x3 = m.x3;
  out.y3 = m.y3;
  out.z3 = m.z3;
  return out;
}

template<typename T, numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<3, 3, T> rotate(const nummat<3, 3, T>& m, U angle) noexcept {
  const U c = std::cos(angle);
  const U s = std::sin(angle);
  nummat<3, 3, T> other;
  other.x1 = static_cast<T>(c);
  other.y1 = static_cast<T>(s);
  other.z1 = T(0);
  other.x2 = -static_cast<T>(s);
  other.y2 = static_cast<T>(c);
  other.z2 = T(0);
  other.x3 = T(0);
  other.y3 = T(0);
  other.z3 = T(1);
  return m * other;
}

template<typename T, numeric_convertible<T> U, bool is_right>
SHOGLE_MATH_DEF nummat<3, 3, T> lookat(const numvec<3, T>& dir, const numvec<3, U>& up) noexcept {
  const numvec<3, T> col3 = is_right ? -dir : dir;
  const numvec<3, T> right = ::shogle::math::cross(::shogle::vec_cast<T>(up), col3);
  const T r2 = ::shogle::math::length2(right);
  const numvec<3, T> col1 = right * ::shogle::math::rsqrt(::shogle::math::max(T(0.000001), r2));
  const numvec<3, T> col2 = ::shogle::math::cross(col3, col1);

  nummat<3, 3, T> out;
  out.x1 = col1.x;
  out.y1 = col1.y;
  out.z1 = col1.z;
  out.x2 = col2.x;
  out.y2 = col2.y;
  out.z2 = col2.z;
  out.x3 = col3.x;
  out.y3 = col3.y;
  out.z3 = col3.z;
  return out;
}

template<typename T, numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<3, 3, T> lookat_rh(const numvec<3, T>& dir,
                                          const numvec<3, U>& up) noexcept {
  return ::shogle::math::lookat<T, U, true>(dir, up);
}

template<typename T, numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<3, 3, T> lookat_lh(const numvec<3, T>& dir,
                                          const numvec<3, U>& up) noexcept {
  return ::shogle::math::lookat<T, U, false>(dir, up);
}

} // namespace shogle::math

namespace shogle {

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<3, 3, T>& nummat<3, 3, T>::operator=(U scalar) noexcept {
  this->x1 = static_cast<T>(scalar);
  this->y1 = static_cast<T>(scalar);
  this->z1 = static_cast<T>(scalar);
  this->x2 = static_cast<T>(scalar);
  this->y2 = static_cast<T>(scalar);
  this->z2 = static_cast<T>(scalar);
  this->x3 = static_cast<T>(scalar);
  this->y3 = static_cast<T>(scalar);
  this->z3 = static_cast<T>(scalar);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<3, 3, T>& nummat<3, 3, T>::operator+=(U scalar) noexcept {
  this->x1 += static_cast<T>(scalar);
  this->y1 += static_cast<T>(scalar);
  this->z1 += static_cast<T>(scalar);
  this->x2 += static_cast<T>(scalar);
  this->y2 += static_cast<T>(scalar);
  this->z2 += static_cast<T>(scalar);
  this->x3 += static_cast<T>(scalar);
  this->y3 += static_cast<T>(scalar);
  this->z3 += static_cast<T>(scalar);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<3, 3, T>& nummat<3, 3, T>::operator-=(U scalar) noexcept {
  this->x1 -= static_cast<T>(scalar);
  this->y1 -= static_cast<T>(scalar);
  this->z1 -= static_cast<T>(scalar);
  this->x2 -= static_cast<T>(scalar);
  this->y2 -= static_cast<T>(scalar);
  this->z2 -= static_cast<T>(scalar);
  this->x3 -= static_cast<T>(scalar);
  this->y3 -= static_cast<T>(scalar);
  this->z3 -= static_cast<T>(scalar);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<3, 3, T>& nummat<3, 3, T>::operator*=(U scalar) noexcept {
  this->x1 *= static_cast<T>(scalar);
  this->y1 *= static_cast<T>(scalar);
  this->z1 *= static_cast<T>(scalar);
  this->x2 *= static_cast<T>(scalar);
  this->y2 *= static_cast<T>(scalar);
  this->z2 *= static_cast<T>(scalar);
  this->x3 *= static_cast<T>(scalar);
  this->y3 *= static_cast<T>(scalar);
  this->z3 *= static_cast<T>(scalar);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<3, 3, T>& nummat<3, 3, T>::operator/=(U scalar) noexcept {
  this->x1 /= static_cast<T>(scalar);
  this->y1 /= static_cast<T>(scalar);
  this->z1 /= static_cast<T>(scalar);
  this->x2 /= static_cast<T>(scalar);
  this->y2 /= static_cast<T>(scalar);
  this->z2 /= static_cast<T>(scalar);
  this->x3 /= static_cast<T>(scalar);
  this->y3 /= static_cast<T>(scalar);
  this->z3 /= static_cast<T>(scalar);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<3, 3, T>&
nummat<3, 3, T>::operator=(const nummat<3, 3, U>& other) noexcept {
  this->x1 = static_cast<T>(other.x1);
  this->y1 = static_cast<T>(other.y1);
  this->z1 = static_cast<T>(other.z1);
  this->x2 = static_cast<T>(other.x2);
  this->y2 = static_cast<T>(other.y2);
  this->z2 = static_cast<T>(other.z2);
  this->x3 = static_cast<T>(other.x3);
  this->y3 = static_cast<T>(other.y3);
  this->z3 = static_cast<T>(other.z3);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<3, 3, T>&
nummat<3, 3, T>::operator+=(const nummat<3, 3, U>& other) noexcept {
  this->x1 += static_cast<T>(other.x1);
  this->y1 += static_cast<T>(other.y1);
  this->z1 += static_cast<T>(other.z1);
  this->x2 += static_cast<T>(other.x2);
  this->y2 += static_cast<T>(other.y2);
  this->z2 += static_cast<T>(other.z2);
  this->x3 += static_cast<T>(other.x3);
  this->y3 += static_cast<T>(other.y3);
  this->z3 += static_cast<T>(other.z3);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<3, 3, T>&
nummat<3, 3, T>::operator-=(const nummat<3, 3, U>& other) noexcept {
  this->x1 -= static_cast<T>(other.x1);
  this->y1 -= static_cast<T>(other.y1);
  this->z1 -= static_cast<T>(other.z1);
  this->x2 -= static_cast<T>(other.x2);
  this->y2 -= static_cast<T>(other.y2);
  this->z2 -= static_cast<T>(other.z2);
  this->x3 -= static_cast<T>(other.x3);
  this->y3 -= static_cast<T>(other.y3);
  this->z3 -= static_cast<T>(other.z3);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<3, 3, T>& nummat<3, 3, T>::operator*=(const nummat<3, 3, U>& m) noexcept {
  const nummat<3, 3, T> self = *this;
  this->x1 = (self.x1 * m.x1) + (self.x2 * m.y1) + (self.x3 * m.z1);
  this->y1 = (self.y1 * m.x1) + (self.y2 * m.y1) + (self.y3 * m.z1);
  this->z1 = (self.z1 * m.x1) + (self.z2 * m.y1) + (self.z3 * m.z1);
  this->x2 = (self.x1 * m.x2) + (self.x2 * m.y2) + (self.x3 * m.z2);
  this->y2 = (self.y1 * m.x2) + (self.y2 * m.y2) + (self.y3 * m.z2);
  this->z2 = (self.z1 * m.x2) + (self.z2 * m.y2) + (self.z3 * m.z2);
  this->x3 = (self.x1 * m.x3) + (self.x2 * m.y3) + (self.x3 * m.z3);
  this->y3 = (self.y1 * m.x3) + (self.y2 * m.y3) + (self.y3 * m.z3);
  this->z3 = (self.z1 * m.x3) + (self.z2 * m.y3) + (self.z3 * m.z3);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<3, 3, T>&
nummat<3, 3, T>::operator/=(const nummat<3, 3, U>& other) noexcept {
  return (*this *= ::shogle::math::inverse(other));
}

template<typename T>
SHOGLE_MATH_DEF bool operator==(const nummat<3, 3, T>& a, const nummat<3, 3, T>& b) noexcept {
  return (a.x1 == b.x1) && (a.y1 == b.y1) && (a.z1 == b.z1) && (a.x2 == b.x2) && (a.y2 == b.y2) &&
         (a.z2 == b.z2) && (a.x3 == b.x3) && (a.y3 == b.y3) && (a.z3 == b.z3);
}

template<typename T>
SHOGLE_MATH_DEF bool operator!=(const nummat<3, 3, T>& a, const nummat<3, 3, T>& b) noexcept {
  return (a.x1 != b.x1) || (a.y1 != b.y1) || (a.z1 != b.z1) || (a.x2 != b.x2) || (a.y2 != b.y2) ||
         (a.z2 != b.z2) || (a.x3 != b.x3) || (a.y3 != b.y3) || (a.z3 != b.z3);
}

template<typename T>
SHOGLE_MATH_DEF nummat<3, 3, T> operator+(const nummat<3, 3, T>& mat) noexcept {
  nummat<3, 3, T> out;
  out.x1 = +mat.x1;
  out.y1 = +mat.y1;
  out.z1 = +mat.z1;
  out.x2 = +mat.x2;
  out.y2 = +mat.y2;
  out.z2 = +mat.z2;
  out.x3 = +mat.x3;
  out.y3 = +mat.y3;
  out.z3 = +mat.z3;
  return out;
}

template<typename T>
SHOGLE_MATH_DEF nummat<3, 3, T> operator+(const nummat<3, 3, T>& a,
                                          const nummat<3, 3, T>& b) noexcept {
  nummat<3, 3, T> out;
  out.x1 = a.x1 + b.x1;
  out.y1 = a.y1 + b.y1;
  out.z1 = a.z1 + b.z1;
  out.x2 = a.x2 + b.x2;
  out.y2 = a.y2 + b.y2;
  out.z2 = a.z2 + b.z2;
  out.x3 = a.x3 + b.x3;
  out.y3 = a.y3 + b.y3;
  out.z3 = a.z3 + b.z3;
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<3, 3, T> operator+(const nummat<3, 3, T>& mat, U scalar) noexcept {
  nummat<3, 3, T> out;
  out.x1 = mat.x1 + static_cast<T>(scalar);
  out.y1 = mat.y1 + static_cast<T>(scalar);
  out.z1 = mat.z1 + static_cast<T>(scalar);
  out.x2 = mat.x2 + static_cast<T>(scalar);
  out.y2 = mat.y2 + static_cast<T>(scalar);
  out.z2 = mat.z2 + static_cast<T>(scalar);
  out.x3 = mat.x3 + static_cast<T>(scalar);
  out.y3 = mat.y3 + static_cast<T>(scalar);
  out.z3 = mat.z3 + static_cast<T>(scalar);
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<3, 3, T> operator+(U scalar, const nummat<3, 3, T>& mat) noexcept {
  nummat<3, 3, T> out;
  out.x1 = static_cast<T>(scalar) + mat.x1;
  out.y1 = static_cast<T>(scalar) + mat.y1;
  out.z1 = static_cast<T>(scalar) + mat.z1;
  out.x2 = static_cast<T>(scalar) + mat.x2;
  out.y2 = static_cast<T>(scalar) + mat.y2;
  out.z2 = static_cast<T>(scalar) + mat.z2;
  out.x3 = static_cast<T>(scalar) + mat.x3;
  out.y3 = static_cast<T>(scalar) + mat.y3;
  out.z3 = static_cast<T>(scalar) + mat.z3;
  return out;
}

template<typename T>
SHOGLE_MATH_DEF nummat<3, 3, T> operator-(const nummat<3, 3, T>& mat) noexcept {
  nummat<3, 3, T> out;
  out.x1 = -mat.x1;
  out.y1 = -mat.y1;
  out.z1 = -mat.z1;
  out.x2 = -mat.x2;
  out.y2 = -mat.y2;
  out.z2 = -mat.z2;
  out.x3 = -mat.x3;
  out.y3 = -mat.y3;
  out.z3 = -mat.z3;
  return out;
}

template<typename T>
SHOGLE_MATH_DEF nummat<3, 3, T> operator-(const nummat<3, 3, T>& a,
                                          const nummat<3, 3, T>& b) noexcept {
  nummat<3, 3, T> out;
  out.x1 = a.x1 - b.x1;
  out.y1 = a.y1 - b.y1;
  out.z1 = a.z1 - b.z1;
  out.x2 = a.x2 - b.x2;
  out.y2 = a.y2 - b.y2;
  out.z2 = a.z2 - b.z2;
  out.x3 = a.x3 - b.x3;
  out.y3 = a.y3 - b.y3;
  out.z3 = a.z3 - b.z3;
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<3, 3, T> operator-(const nummat<3, 3, T>& mat, U scalar) noexcept {
  nummat<3, 3, T> out;
  out.x1 = mat.x1 - static_cast<T>(scalar);
  out.y1 = mat.y1 - static_cast<T>(scalar);
  out.z1 = mat.z1 - static_cast<T>(scalar);
  out.x2 = mat.x2 - static_cast<T>(scalar);
  out.y2 = mat.y2 - static_cast<T>(scalar);
  out.z2 = mat.z2 - static_cast<T>(scalar);
  out.x3 = mat.x3 - static_cast<T>(scalar);
  out.y3 = mat.y3 - static_cast<T>(scalar);
  out.z3 = mat.z3 - static_cast<T>(scalar);
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<3, 3, T> operator-(U scalar, const nummat<3, 3, T>& mat) noexcept {
  nummat<3, 3, T> out;
  out.x1 = static_cast<T>(scalar) - mat.x1;
  out.y1 = static_cast<T>(scalar) - mat.y1;
  out.z1 = static_cast<T>(scalar) - mat.z1;
  out.x2 = static_cast<T>(scalar) - mat.x2;
  out.y2 = static_cast<T>(scalar) - mat.y2;
  out.z2 = static_cast<T>(scalar) - mat.z2;
  out.x3 = static_cast<T>(scalar) - mat.x3;
  out.y3 = static_cast<T>(scalar) - mat.y3;
  out.z3 = static_cast<T>(scalar) - mat.z3;
  return out;
}

template<typename T>
SHOGLE_MATH_DEF nummat<3, 3, T> operator*(const nummat<3, 3, T>& a,
                                          const nummat<3, 3, T>& b) noexcept {
  nummat<3, 3, T> out;
  out.x1 = (a.x1 * b.x1) + (a.x2 * b.y1) + (a.x3 * b.z1);
  out.y1 = (a.y1 * b.x1) + (a.y2 * b.y1) + (a.y3 * b.z1);
  out.z1 = (a.z1 * b.x1) + (a.z2 * b.y1) + (a.z3 * b.z1);
  out.x2 = (a.x1 * b.x2) + (a.x2 * b.y2) + (a.x3 * b.z2);
  out.y2 = (a.y1 * b.x2) + (a.y2 * b.y2) + (a.y3 * b.z2);
  out.z2 = (a.z1 * b.x2) + (a.z2 * b.y2) + (a.z3 * b.z2);
  out.x3 = (a.x1 * b.x3) + (a.x2 * b.y3) + (a.x3 * b.z3);
  out.y3 = (a.y1 * b.x3) + (a.y2 * b.y3) + (a.y3 * b.z3);
  out.z3 = (a.z1 * b.x3) + (a.z2 * b.y3) + (a.z3 * b.z3);
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<3, 3, T> operator*(const nummat<3, 3, T>& mat, U scalar) noexcept {
  nummat<3, 3, T> out;
  out.x1 = mat.x1 * static_cast<T>(scalar);
  out.y1 = mat.y1 * static_cast<T>(scalar);
  out.z1 = mat.z1 * static_cast<T>(scalar);
  out.x2 = mat.x2 * static_cast<T>(scalar);
  out.y2 = mat.y2 * static_cast<T>(scalar);
  out.z2 = mat.z2 * static_cast<T>(scalar);
  out.x3 = mat.x3 * static_cast<T>(scalar);
  out.y3 = mat.y3 * static_cast<T>(scalar);
  out.z3 = mat.z3 * static_cast<T>(scalar);
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<3, 3, T> operator*(U scalar, const nummat<3, 3, T>& mat) noexcept {
  nummat<3, 3, T> out;
  out.x1 = static_cast<T>(scalar) * mat.x1;
  out.y1 = static_cast<T>(scalar) * mat.y1;
  out.z1 = static_cast<T>(scalar) * mat.z1;
  out.x2 = static_cast<T>(scalar) * mat.x2;
  out.y2 = static_cast<T>(scalar) * mat.y2;
  out.z2 = static_cast<T>(scalar) * mat.z2;
  out.x3 = static_cast<T>(scalar) * mat.x3;
  out.y3 = static_cast<T>(scalar) * mat.y3;
  out.z3 = static_cast<T>(scalar) * mat.z3;
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF typename nummat<3, 3, T>::col_type operator*(const nummat<3, 3, T>& m,
                                                             const numvec<3, U>& v) noexcept {
  typename nummat<3, 3, T>::col_type out;
  out.x = (m.x1 * v.x) + (m.x2 * v.y) + (m.x3 * v.z);
  out.y = (m.y1 * v.x) + (m.y2 * v.y) + (m.y3 * v.z);
  out.z = (m.z1 * v.x) + (m.z2 * v.y) + (m.z3 * v.z);
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF typename nummat<3, 3, T>::row_type operator*(const numvec<3, U>& v,
                                                             const nummat<3, 3, T>& m) noexcept {
  typename nummat<3, 3, T>::row_type out;
  out.x = (v.x * m.x1) + (v.y * m.y1) + (v.z * m.z1);
  out.y = (v.x * m.x2) + (v.y * m.y2) + (v.z * m.z2);
  out.z = (v.x * m.x3) + (v.y * m.y3) + (v.z * m.z3);
  return out;
}

template<typename T>
SHOGLE_MATH_DEF nummat<3, 3, T> operator/(const nummat<3, 3, T>& a,
                                          const nummat<3, 3, T>& b) noexcept {
  return a * ::shogle::math::inverse(b);
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<3, 3, T> operator/(const nummat<3, 3, T>& mat, U scalar) noexcept {
  nummat<3, 3, T> out;
  out.x1 = mat.x1 / static_cast<T>(scalar);
  out.y1 = mat.y1 / static_cast<T>(scalar);
  out.z1 = mat.z1 / static_cast<T>(scalar);
  out.x2 = mat.x2 / static_cast<T>(scalar);
  out.y2 = mat.y2 / static_cast<T>(scalar);
  out.z2 = mat.z2 / static_cast<T>(scalar);
  out.x3 = mat.x3 / static_cast<T>(scalar);
  out.y3 = mat.y3 / static_cast<T>(scalar);
  out.z3 = mat.z3 / static_cast<T>(scalar);
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<3, 3, T> operator/(U scalar, const nummat<3, 3, T>& mat) noexcept {
  nummat<3, 3, T> out;
  out.x1 = static_cast<T>(scalar) / mat.x1;
  out.y1 = static_cast<T>(scalar) / mat.y1;
  out.z1 = static_cast<T>(scalar) / mat.z1;
  out.x2 = static_cast<T>(scalar) / mat.x2;
  out.y2 = static_cast<T>(scalar) / mat.y2;
  out.z2 = static_cast<T>(scalar) / mat.z2;
  out.x3 = static_cast<T>(scalar) / mat.x3;
  out.y3 = static_cast<T>(scalar) / mat.y3;
  out.z3 = static_cast<T>(scalar) / mat.z3;
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF typename nummat<3, 3, T>::col_type operator/(const nummat<3, 3, T>& mat,
                                                             const numvec<3, U>& vec) noexcept {
  return ::shogle::math::inverse(mat) * vec;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF typename nummat<3, 3, T>::row_type operator/(const numvec<3, U>& vec,
                                                             const nummat<3, 3, T>& mat) noexcept {
  return vec * ::shogle::math::inverse(mat);
}

} // namespace shogle
