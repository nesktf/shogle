#define SHOGLE_MATH_MATRIX4x4_INL
#include "./matrix4x4.hpp"
#undef SHOGLE_MATH_MATRIX4x4_INL

namespace shogle::math {

template<typename T>
SHOGLE_MATH_DEF T determinant(const nummat<4, 4, T>& m) noexcept {
  const T t0 = (m.y2 * (m.z3 * m.w4 - m.z4 * m.w3) - m.z2 * (m.y3 * m.w4 - m.y4 * m.w3) +
                m.w2 * (m.y3 * m.z4 - m.y4 * m.z3));
  const T t1 = -(m.y1 * (m.z3 * m.w4 - m.z4 * m.w3) - m.z1 * (m.y3 * m.w4 - m.y4 * m.w3) +
                 m.w1 * (m.y3 * m.z4 - m.y4 * m.z3));
  const T t2 = (m.y1 * (m.z2 * m.w4 - m.z4 * m.w2) - m.z1 * (m.y2 * m.w4 - m.y4 * m.w2) +
                m.w1 * (m.y2 * m.z4 - m.y4 * m.z2));
  const T t3 = -(m.y1 * (m.z2 * m.w3 - m.z3 * m.w2) - m.z1 * (m.y2 * m.w3 - m.y3 * m.w2) +
                 m.w1 * (m.y2 * m.z3 - m.y3 * m.z2));
  return m.x1 * t0 + m.x2 * t1 + m.x3 * t2 + m.x4 * t3;
}

template<typename T>
SHOGLE_MATH_DEF nummat<4, 4, T> inverse(const nummat<4, 4, T>& m) noexcept {
  const T c01 = m.y2 * (m.z3 * m.w4 - m.z4 * m.w3) - m.z2 * (m.y3 * m.w4 - m.y4 * m.w3) +
                m.w2 * (m.y3 * m.z4 - m.y4 * m.z3);
  const T c02 = -(m.y1 * (m.z3 * m.w4 - m.z4 * m.w3) - m.z1 * (m.y3 * m.w4 - m.y4 * m.w3) +
                  m.w1 * (m.y3 * m.z4 - m.y4 * m.z3));
  const T c03 = m.y1 * (m.z2 * m.w4 - m.z4 * m.w2) - m.z1 * (m.y2 * m.w4 - m.y4 * m.w2) +
                m.w1 * (m.y2 * m.z4 - m.y4 * m.z2);
  const T c04 = -(m.y1 * (m.z2 * m.w3 - m.z3 * m.w2) - m.z1 * (m.y2 * m.w3 - m.y3 * m.w2) +
                  m.w1 * (m.y2 * m.z3 - m.y3 * m.z2));

  const T invdet = static_cast<T>(1) / (m.x1 * c01 + m.x2 * c02 + m.x3 * c03 + m.x4 * c04);
  nummat<4, 4, T> out;
  out.x1 = c01 * invdet;
  out.y1 = c02 * invdet;
  out.z1 = c03 * invdet;
  out.w1 = c04 * invdet;
  out.x2 = -(m.x2 * (m.z3 * m.w4 - m.z4 * m.w3) - m.z2 * (m.x3 * m.w4 - m.x4 * m.w3) +
             m.w2 * (m.x3 * m.z4 - m.x4 * m.z3)) *
           invdet;
  out.y2 = (m.x1 * (m.z3 * m.w4 - m.z4 * m.w3) - m.z1 * (m.x3 * m.w4 - m.x4 * m.w3) +
            m.w1 * (m.x3 * m.z4 - m.x4 * m.z3)) *
           invdet;
  out.z2 = -(m.x1 * (m.z2 * m.w4 - m.z4 * m.w2) - m.z1 * (m.x2 * m.w4 - m.x4 * m.w2) +
             m.w1 * (m.x2 * m.z4 - m.x4 * m.z2)) *
           invdet;
  out.w2 = (m.x1 * (m.z2 * m.w3 - m.z3 * m.w2) - m.z1 * (m.x2 * m.w3 - m.x3 * m.w2) +
            m.w1 * (m.x2 * m.z3 - m.x3 * m.z2)) *
           invdet;
  out.x3 = (m.x2 * (m.y3 * m.w4 - m.y4 * m.w3) - m.y2 * (m.x3 * m.w4 - m.x4 * m.w3) +
            m.w2 * (m.x3 * m.y4 - m.x4 * m.y3)) *
           invdet;
  out.y3 = -(m.x1 * (m.y3 * m.w4 - m.y4 * m.w3) - m.y1 * (m.x3 * m.w4 - m.x4 * m.w3) +
             m.w1 * (m.x3 * m.y4 - m.x4 * m.y3)) *
           invdet;
  out.z3 = (m.x1 * (m.y2 * m.w4 - m.y4 * m.w2) - m.y1 * (m.x2 * m.w4 - m.x4 * m.w2) +
            m.w1 * (m.x2 * m.y4 - m.x4 * m.y2)) *
           invdet;
  out.w3 = -(m.x1 * (m.y2 * m.w3 - m.y3 * m.w2) - m.y1 * (m.x2 * m.w3 - m.x3 * m.y2) +
             m.w1 * (m.x2 * m.y3 - m.x3 * m.y2)) *
           invdet;
  out.x4 = -(m.x2 * (m.y3 * m.z4 - m.y4 * m.z3) - m.y2 * (m.x3 * m.z4 - m.x4 * m.z3) +
             m.z2 * (m.x3 * m.y4 - m.x4 * m.y3)) *
           invdet;
  out.y4 = (m.x1 * (m.y3 * m.z4 - m.y4 * m.z3) - m.y1 * (m.x3 * m.z4 - m.x4 * m.z3) +
            m.z1 * (m.x3 * m.y4 - m.x4 * m.y3)) *
           invdet;
  out.z4 = -(m.x1 * (m.y2 * m.z4 - m.y4 * m.z2) - m.y1 * (m.x2 * m.z4 - m.x4 * m.z2) +
             m.z1 * (m.x2 * m.y4 - m.x4 * m.y2)) *
           invdet;
  out.w4 = (m.x1 * (m.y2 * m.z3 - m.y3 * m.z2) - m.y1 * (m.x2 * m.z3 - m.x3 * m.z2) +
            m.z1 * (m.x2 * m.y3 - m.x3 * m.y2)) *
           invdet;
  return out;
}

template<typename T>
SHOGLE_MATH_DEF nummat<4, 4, T> transpose(const nummat<4, 4, T>& m) noexcept {
  nummat<4, 4, T> out;
  out.x1 = m.x1;
  out.y1 = m.x2;
  out.z1 = m.x3;
  out.w1 = m.x4;
  out.x2 = m.y1;
  out.y2 = m.y2;
  out.z2 = m.y3;
  out.w2 = m.y4;
  out.x3 = m.z1;
  out.y3 = m.z2;
  out.z3 = m.z3;
  out.w3 = m.z4;
  out.x4 = m.w1;
  out.y4 = m.w2;
  out.z4 = m.w3;
  out.w4 = m.w4;
  return out;
}

template<typename T, numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<4, 4, T> translate(const nummat<4, 4, T>& m,
                                          const numvec<3, U>& v) noexcept {
  nummat<4, 4, T> out = m;
  out.x4 = (m.x1 * static_cast<T>(v.x)) + (m.x2 * static_cast<T>(v.y)) +
           (m.x3 * static_cast<T>(v.z)) + m.x4;
  out.y4 = (m.y1 * static_cast<T>(v.x)) + (m.y2 * static_cast<T>(v.y)) +
           (m.y3 * static_cast<T>(v.z)) + m.y4;
  out.z4 = (m.z1 * static_cast<T>(v.x)) + (m.z2 * static_cast<T>(v.y)) +
           (m.z3 * static_cast<T>(v.z)) + m.z4;
  out.w4 = (m.w1 * static_cast<T>(v.x)) + (m.w2 * static_cast<T>(v.y)) +
           (m.w3 * static_cast<T>(v.z)) + m.w4;
  return out;
}

template<typename T, numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<4, 4, T> scale(const nummat<4, 4, T>& m, const numvec<3, U>& v) noexcept {
  nummat<4, 4, T> out;
  out.x1 = m.x1 * static_cast<T>(v.x);
  out.y1 = m.y1 * static_cast<T>(v.x);
  out.z1 = m.z1 * static_cast<T>(v.x);
  out.x2 = m.x2 * static_cast<T>(v.y);
  out.y2 = m.y2 * static_cast<T>(v.y);
  out.z2 = m.z2 * static_cast<T>(v.y);
  out.x3 = m.x3 * static_cast<T>(v.z);
  out.y3 = m.y3 * static_cast<T>(v.z);
  out.z3 = m.z3 * static_cast<T>(v.z);
  out.x4 = m.x4;
  out.y4 = m.y4;
  out.z4 = m.z4;
  return out;
}

template<typename T, numeric_convertible<T> U, numeric_convertible<T> V>
SHOGLE_MATH_DEF nummat<4, 4, T> rotate(const nummat<4, 4, T>& m, U angle,
                                       const numvec<3, V>& axis) noexcept {
  const T c = static_cast<T>(std::cos(angle));
  const T s = static_cast<T>(std::sin(angle));

  const auto ax = ::shogle::vec_cast<T>(::shogle::math::normalize(axis));
  const auto inv_c = (ax * (T(1) - c));

  struct {
    T x1, y1, z1;
    T x2, y2, z2;
    T x3, y3, z3;
  } rot; // temporary matrix, just to avoid including matrix3x3

  rot.x1 = c + (inv_c.x * ax.x);
  rot.y1 = (inv_c.x * ax.y) + (s * ax.z);
  rot.z1 = (inv_c.x * ax.z) - (s * ax.y);
  rot.x2 = (inv_c.y * ax.x) - (s * ax.z);
  rot.y2 = c + (inv_c.y * ax.y);
  rot.z2 = (inv_c.y * ax.z) + (s * ax.x);
  rot.x3 = (inv_c.z * ax.x) + (s * ax.y);
  rot.y3 = (inv_c.z * ax.y) - (s * ax.x);
  rot.z3 = c + (inv_c.z * ax.z);

  nummat<4, 4, T> out;
  out.x1 = (m.x1 * rot.x1) + (m.x2 * rot.y1) + (m.x3 * rot.z1);
  out.y1 = (m.y1 * rot.x1) + (m.y2 * rot.y1) + (m.y3 * rot.z1);
  out.z1 = (m.z1 * rot.x1) + (m.z2 * rot.y1) + (m.z3 * rot.z1);
  out.x2 = (m.x1 * rot.x2) + (m.x2 * rot.y2) + (m.x3 * rot.z2);
  out.y2 = (m.y1 * rot.x2) + (m.y2 * rot.y2) + (m.y3 * rot.z2);
  out.z2 = (m.z1 * rot.x2) + (m.z2 * rot.y2) + (m.z3 * rot.z2);
  out.x2 = (m.x1 * rot.x3) + (m.x2 * rot.y3) + (m.x3 * rot.z3);
  out.y2 = (m.y1 * rot.x3) + (m.y2 * rot.y3) + (m.y3 * rot.z3);
  out.z2 = (m.z1 * rot.x3) + (m.z2 * rot.y3) + (m.z3 * rot.z3);
  out.x4 = m.x4;
  out.y4 = m.y4;
  out.z4 = m.z4;
  out.w4 = m.w4;
  return out;
}

template<typename T>
SHOGLE_MATH_DEF nummat<3, 3, T> to_mat3(const nummat<4, 4, T>& m) noexcept {
  nummat<3, 3, T> out;
  out.x1 = m.x1;
  out.y1 = m.y1;
  out.z1 = m.z1;
  out.x2 = m.x2;
  out.y2 = m.y2;
  out.z2 = m.z2;
  out.x3 = m.x3;
  out.y3 = m.y3;
  out.z3 = m.z3;
  return out;
}

template<typename T>
SHOGLE_MATH_DEF nummat<4, 4, T> to_mat4(const nummat<3, 3, T>& m) noexcept {
  nummat<4, 4, T> out;
  out.x1 = m.x1;
  out.y1 = m.y1;
  out.z1 = m.z1;
  out.w1 = T(0);
  out.x2 = m.x2;
  out.y2 = m.y2;
  out.z2 = m.z2;
  out.w2 = T(0);
  out.x3 = m.x3;
  out.y3 = m.y3;
  out.z3 = m.z3;
  out.w3 = T(0);
  out.x4 = T(0);
  out.y4 = T(0);
  out.z4 = T(0);
  out.w4 = T(1);
  return out;
}

template<typename T>
SHOGLE_MATH_DEF nummat<4, 4, T> lookat_rh(const numvec<3, T>& pos, const numvec<3, T>& center,
                                          const numvec<3, T>& up) noexcept {
  const numvec<3, T> row3 = ::shogle::math::normalize(center - pos);
  const numvec<3, T> row1 = ::shogle::math::normalize(::shogle::math::cross(row3, up));
  const numvec<3, T> row2 = ::shogle::math::cross(row1, row3);

  nummat<4, 4, T> out;
  out.x1 = row1.x;
  out.x2 = row1.y;
  out.x3 = row1.z;
  out.x4 = -::shogle::math::dot(row1, pos);
  out.y1 = row2.x;
  out.y2 = row2.y;
  out.y3 = row2.z;
  out.y4 = -shogle::math::dot(row2, pos);
  out.z1 = -row3.x;
  out.z2 = -row3.y;
  out.z3 = -row3.z;
  out.z4 = ::shogle::math::dot(row3, pos);
  out.w1 = T(0);
  out.w2 = T(0);
  out.w3 = T(0);
  out.w4 = T(1);
  return out;
}

template<typename T>
SHOGLE_MATH_DEF nummat<4, 4, T> lookat_lh(const numvec<3, T>& pos, const numvec<3, T>& center,
                                          const numvec<3, T>& up) noexcept {
  const numvec<3, T> row3 = ::shogle::math::normalize(center - pos);
  const numvec<3, T> row1 = ::shogle::math::normalize(::shogle::math::cross(row3, up));
  const numvec<3, T> row2 = ::shogle::math::cross(row1, row3);

  nummat<4, 4, T> out;
  out.x1 = row1.x;
  out.x2 = row1.y;
  out.x3 = row1.z;
  out.x4 = -::shogle::math::dot(row1, pos);
  out.y1 = row2.x;
  out.y2 = row2.y;
  out.y3 = row2.z;
  out.y4 = -shogle::math::dot(row2, pos);
  out.z1 = row3.x;
  out.z2 = row3.y;
  out.z3 = row3.z;
  out.z4 = -::shogle::math::dot(row3, pos);
  out.w1 = T(0);
  out.w2 = T(0);
  out.w3 = T(0);
  out.w4 = T(1);
  return out;
}

} // namespace shogle::math

namespace shogle {

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<4, 4, T>& nummat<4, 4, T>::operator=(U scalar) noexcept {
  this->x1 = static_cast<T>(scalar);
  this->y1 = static_cast<T>(scalar);
  this->z1 = static_cast<T>(scalar);
  this->w1 = static_cast<T>(scalar);
  this->x2 = static_cast<T>(scalar);
  this->y2 = static_cast<T>(scalar);
  this->z2 = static_cast<T>(scalar);
  this->w2 = static_cast<T>(scalar);
  this->x3 = static_cast<T>(scalar);
  this->y3 = static_cast<T>(scalar);
  this->z3 = static_cast<T>(scalar);
  this->w3 = static_cast<T>(scalar);
  this->x4 = static_cast<T>(scalar);
  this->y4 = static_cast<T>(scalar);
  this->z4 = static_cast<T>(scalar);
  this->w4 = static_cast<T>(scalar);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<4, 4, T>& nummat<4, 4, T>::operator+=(U scalar) noexcept {
  this->x1 += static_cast<T>(scalar);
  this->y1 += static_cast<T>(scalar);
  this->z1 += static_cast<T>(scalar);
  this->w1 += static_cast<T>(scalar);
  this->x2 += static_cast<T>(scalar);
  this->y2 += static_cast<T>(scalar);
  this->z2 += static_cast<T>(scalar);
  this->w2 += static_cast<T>(scalar);
  this->x3 += static_cast<T>(scalar);
  this->y3 += static_cast<T>(scalar);
  this->z3 += static_cast<T>(scalar);
  this->w3 += static_cast<T>(scalar);
  this->x4 += static_cast<T>(scalar);
  this->y4 += static_cast<T>(scalar);
  this->z4 += static_cast<T>(scalar);
  this->w4 += static_cast<T>(scalar);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<4, 4, T>& nummat<4, 4, T>::operator-=(U scalar) noexcept {
  this->x1 -= static_cast<T>(scalar);
  this->y1 -= static_cast<T>(scalar);
  this->z1 -= static_cast<T>(scalar);
  this->w1 -= static_cast<T>(scalar);
  this->x2 -= static_cast<T>(scalar);
  this->y2 -= static_cast<T>(scalar);
  this->z2 -= static_cast<T>(scalar);
  this->w2 -= static_cast<T>(scalar);
  this->x3 -= static_cast<T>(scalar);
  this->y3 -= static_cast<T>(scalar);
  this->z3 -= static_cast<T>(scalar);
  this->w3 -= static_cast<T>(scalar);
  this->x4 -= static_cast<T>(scalar);
  this->y4 -= static_cast<T>(scalar);
  this->z4 -= static_cast<T>(scalar);
  this->w4 -= static_cast<T>(scalar);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<4, 4, T>& nummat<4, 4, T>::operator*=(U scalar) noexcept {
  this->x1 *= static_cast<T>(scalar);
  this->y1 *= static_cast<T>(scalar);
  this->z1 *= static_cast<T>(scalar);
  this->w1 *= static_cast<T>(scalar);
  this->x2 *= static_cast<T>(scalar);
  this->y2 *= static_cast<T>(scalar);
  this->z2 *= static_cast<T>(scalar);
  this->w2 *= static_cast<T>(scalar);
  this->x3 *= static_cast<T>(scalar);
  this->y3 *= static_cast<T>(scalar);
  this->z3 *= static_cast<T>(scalar);
  this->w3 *= static_cast<T>(scalar);
  this->x4 *= static_cast<T>(scalar);
  this->y4 *= static_cast<T>(scalar);
  this->z4 *= static_cast<T>(scalar);
  this->w4 *= static_cast<T>(scalar);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<4, 4, T>& nummat<4, 4, T>::operator/=(U scalar) noexcept {
  this->x1 /= static_cast<T>(scalar);
  this->y1 /= static_cast<T>(scalar);
  this->z1 /= static_cast<T>(scalar);
  this->w1 /= static_cast<T>(scalar);
  this->x2 /= static_cast<T>(scalar);
  this->y2 /= static_cast<T>(scalar);
  this->z2 /= static_cast<T>(scalar);
  this->w2 /= static_cast<T>(scalar);
  this->x3 /= static_cast<T>(scalar);
  this->y3 /= static_cast<T>(scalar);
  this->z3 /= static_cast<T>(scalar);
  this->w3 /= static_cast<T>(scalar);
  this->x4 /= static_cast<T>(scalar);
  this->y4 /= static_cast<T>(scalar);
  this->z4 /= static_cast<T>(scalar);
  this->w4 /= static_cast<T>(scalar);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<4, 4, T>&
nummat<4, 4, T>::operator=(const nummat<4, 4, U>& other) noexcept {
  this->x1 = static_cast<T>(other.x1);
  this->y1 = static_cast<T>(other.y1);
  this->z1 = static_cast<T>(other.z1);
  this->w1 = static_cast<T>(other.w1);
  this->x2 = static_cast<T>(other.x2);
  this->y2 = static_cast<T>(other.y2);
  this->z2 = static_cast<T>(other.z2);
  this->w2 = static_cast<T>(other.w2);
  this->x3 = static_cast<T>(other.x3);
  this->y3 = static_cast<T>(other.y3);
  this->z3 = static_cast<T>(other.z3);
  this->w3 = static_cast<T>(other.w3);
  this->x4 = static_cast<T>(other.x4);
  this->y4 = static_cast<T>(other.y4);
  this->z4 = static_cast<T>(other.z4);
  this->w4 = static_cast<T>(other.w4);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<4, 4, T>&
nummat<4, 4, T>::operator+=(const nummat<4, 4, U>& other) noexcept {
  this->x1 += static_cast<T>(other.x1);
  this->y1 += static_cast<T>(other.y1);
  this->z1 += static_cast<T>(other.z1);
  this->w1 += static_cast<T>(other.w1);
  this->x2 += static_cast<T>(other.x2);
  this->y2 += static_cast<T>(other.y2);
  this->z2 += static_cast<T>(other.z2);
  this->w2 += static_cast<T>(other.w2);
  this->x3 += static_cast<T>(other.x3);
  this->y3 += static_cast<T>(other.y3);
  this->z3 += static_cast<T>(other.z3);
  this->w3 += static_cast<T>(other.w3);
  this->x4 += static_cast<T>(other.x4);
  this->y4 += static_cast<T>(other.y4);
  this->z4 += static_cast<T>(other.z4);
  this->w4 += static_cast<T>(other.w4);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<4, 4, T>&
nummat<4, 4, T>::operator-=(const nummat<4, 4, U>& other) noexcept {
  this->x1 -= static_cast<T>(other.x1);
  this->y1 -= static_cast<T>(other.y1);
  this->z1 -= static_cast<T>(other.z1);
  this->w1 -= static_cast<T>(other.w1);
  this->x2 -= static_cast<T>(other.x2);
  this->y2 -= static_cast<T>(other.y2);
  this->z2 -= static_cast<T>(other.z2);
  this->w2 -= static_cast<T>(other.w2);
  this->x3 -= static_cast<T>(other.x3);
  this->y3 -= static_cast<T>(other.y3);
  this->z3 -= static_cast<T>(other.z3);
  this->w3 -= static_cast<T>(other.w3);
  this->x4 -= static_cast<T>(other.x4);
  this->y4 -= static_cast<T>(other.y4);
  this->z4 -= static_cast<T>(other.z4);
  this->w4 -= static_cast<T>(other.w4);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<4, 4, T>& nummat<4, 4, T>::operator*=(const nummat<4, 4, U>& m) noexcept {
  const nummat<4, 4, T> self = *this;
  this->x1 = (self.x1 * m.x1) + (self.x2 * m.y1) + (self.x3 * m.z1) + (self.x4 * m.w1);
  this->y1 = (self.y1 * m.x1) + (self.y2 * m.y1) + (self.y3 * m.z1) + (self.y4 * m.w1);
  this->z1 = (self.z1 * m.x1) + (self.z2 * m.y1) + (self.z3 * m.z1) + (self.z4 * m.w1);
  this->w1 = (self.w1 * m.x1) + (self.w2 * m.y1) + (self.w3 * m.z1) + (self.w4 * m.w1);
  this->x2 = (self.x1 * m.x2) + (self.x2 * m.y2) + (self.x3 * m.z2) + (self.x4 * m.w2);
  this->y2 = (self.y1 * m.x2) + (self.y2 * m.y2) + (self.y3 * m.z2) + (self.y4 * m.w2);
  this->z2 = (self.z1 * m.x2) + (self.z2 * m.y2) + (self.z3 * m.z2) + (self.z4 * m.w2);
  this->w2 = (self.w1 * m.x2) + (self.w2 * m.y2) + (self.w3 * m.z2) + (self.w4 * m.w2);
  this->x3 = (self.x1 * m.x3) + (self.x2 * m.y3) + (self.x3 * m.z3) + (self.x4 * m.w3);
  this->y3 = (self.y1 * m.x3) + (self.y2 * m.y3) + (self.y3 * m.z3) + (self.y4 * m.w3);
  this->z3 = (self.z1 * m.x3) + (self.z2 * m.y3) + (self.z3 * m.z3) + (self.z4 * m.w3);
  this->w3 = (self.w1 * m.x3) + (self.w2 * m.y3) + (self.w3 * m.z3) + (self.w4 * m.w3);
  this->x4 = (self.x1 * m.x4) + (self.x2 * m.y4) + (self.x3 * m.z4) + (self.x4 * m.w4);
  this->y4 = (self.y1 * m.x4) + (self.y2 * m.y4) + (self.y3 * m.z4) + (self.y4 * m.w4);
  this->z4 = (self.z1 * m.x4) + (self.z2 * m.y4) + (self.z3 * m.z4) + (self.z4 * m.w4);
  this->w4 = (self.w1 * m.x4) + (self.w2 * m.y4) + (self.w3 * m.z4) + (self.w4 * m.w4);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<4, 4, T>&
nummat<4, 4, T>::operator/=(const nummat<4, 4, U>& other) noexcept {
  return (*this *= ::shogle::math::inverse(other));
}

template<typename T>
SHOGLE_MATH_DEF bool operator==(const nummat<4, 4, T>& a, const nummat<4, 4, T>& b) noexcept {
  return (a.x1 == b.x1) && (a.y1 == b.y1) && (a.z1 == b.z1) && (a.w1 == b.w1) && (a.x2 == b.x2) &&
         (a.y2 == b.y2) && (a.z2 == b.z2) && (a.w2 == b.w2) && (a.x3 == b.x3) && (a.y3 == b.y3) &&
         (a.z3 == b.z3) && (a.w3 == b.w3) && (a.x4 == b.x4) && (a.y4 == b.y4) && (a.z4 == b.z4) &&
         (a.w4 == b.w4);
}

template<typename T>
SHOGLE_MATH_DEF bool operator!=(const nummat<4, 4, T>& a, const nummat<4, 4, T>& b) noexcept {
  return (a.x1 != b.x1) || (a.y1 != b.y1) || (a.z1 != b.z1) || (a.w1 != b.w1) || (a.x2 != b.x2) ||
         (a.y2 != b.y2) || (a.z2 != b.z2) || (a.w2 != b.w2) || (a.x3 != b.x3) || (a.y3 != b.y3) ||
         (a.z3 != b.z3) || (a.w3 != b.w3) || (a.x4 != b.x4) || (a.y4 != b.y4) || (a.z4 != b.z4) ||
         (a.w4 != b.w4);
}

template<typename T>
SHOGLE_MATH_DEF nummat<4, 4, T> operator+(const nummat<4, 4, T>& mat) noexcept {
  nummat<4, 4, T> out;
  out.x1 = +mat.x1;
  out.y1 = +mat.y1;
  out.z1 = +mat.z1;
  out.w1 = +mat.w1;
  out.x2 = +mat.x2;
  out.y2 = +mat.y2;
  out.z2 = +mat.z2;
  out.w2 = +mat.w2;
  out.x3 = +mat.x3;
  out.y3 = +mat.y3;
  out.z3 = +mat.z3;
  out.w3 = +mat.w3;
  out.x4 = +mat.x4;
  out.y4 = +mat.y4;
  out.z4 = +mat.z4;
  out.w4 = +mat.w4;
  return out;
}

template<typename T>
SHOGLE_MATH_DEF nummat<4, 4, T> operator+(const nummat<4, 4, T>& a,
                                          const nummat<4, 4, T>& b) noexcept {
  nummat<4, 4, T> out;
  out.x1 = a.x1 + b.x1;
  out.y1 = a.y1 + b.y1;
  out.z1 = a.z1 + b.z1;
  out.w1 = a.w1 + b.w1;
  out.x2 = a.x2 + b.x2;
  out.y2 = a.y2 + b.y2;
  out.z2 = a.z2 + b.z2;
  out.w2 = a.w2 + b.w2;
  out.x3 = a.x3 + b.x3;
  out.y3 = a.y3 + b.y3;
  out.z3 = a.z3 + b.z3;
  out.w3 = a.w3 + b.w3;
  out.x4 = a.x4 + b.x4;
  out.y4 = a.y4 + b.y4;
  out.z4 = a.z4 + b.z4;
  out.w4 = a.w4 + b.w4;
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<4, 4, T> operator+(const nummat<4, 4, T>& mat, U scalar) noexcept {
  nummat<4, 4, T> out;
  out.x1 = mat.x1 + static_cast<T>(scalar);
  out.y1 = mat.y1 + static_cast<T>(scalar);
  out.z1 = mat.z1 + static_cast<T>(scalar);
  out.w1 = mat.w1 + static_cast<T>(scalar);
  out.x2 = mat.x2 + static_cast<T>(scalar);
  out.y2 = mat.y2 + static_cast<T>(scalar);
  out.z2 = mat.z2 + static_cast<T>(scalar);
  out.w2 = mat.w2 + static_cast<T>(scalar);
  out.x3 = mat.x3 + static_cast<T>(scalar);
  out.y3 = mat.y3 + static_cast<T>(scalar);
  out.z3 = mat.z3 + static_cast<T>(scalar);
  out.w3 = mat.w3 + static_cast<T>(scalar);
  out.x4 = mat.x4 + static_cast<T>(scalar);
  out.y4 = mat.y4 + static_cast<T>(scalar);
  out.z4 = mat.z4 + static_cast<T>(scalar);
  out.w4 = mat.w4 + static_cast<T>(scalar);
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<4, 4, T> operator+(U scalar, const nummat<4, 4, T>& mat) noexcept {
  nummat<4, 4, T> out;
  out.x1 = static_cast<T>(scalar) + mat.x1;
  out.y1 = static_cast<T>(scalar) + mat.y1;
  out.z1 = static_cast<T>(scalar) + mat.z1;
  out.w1 = static_cast<T>(scalar) + mat.w1;
  out.x2 = static_cast<T>(scalar) + mat.x2;
  out.y2 = static_cast<T>(scalar) + mat.y2;
  out.z2 = static_cast<T>(scalar) + mat.z2;
  out.w2 = static_cast<T>(scalar) + mat.w2;
  out.x3 = static_cast<T>(scalar) + mat.x3;
  out.y3 = static_cast<T>(scalar) + mat.y3;
  out.z3 = static_cast<T>(scalar) + mat.z3;
  out.w3 = static_cast<T>(scalar) + mat.w3;
  out.x4 = static_cast<T>(scalar) + mat.x4;
  out.y4 = static_cast<T>(scalar) + mat.y4;
  out.z4 = static_cast<T>(scalar) + mat.z4;
  out.w4 = static_cast<T>(scalar) + mat.w4;
  return out;
}

template<typename T>
SHOGLE_MATH_DEF nummat<4, 4, T> operator-(const nummat<4, 4, T>& mat) noexcept {
  nummat<4, 4, T> out;
  out.x1 = -mat.x1;
  out.y1 = -mat.y1;
  out.z1 = -mat.z1;
  out.w1 = -mat.w1;
  out.x2 = -mat.x2;
  out.y2 = -mat.y2;
  out.z2 = -mat.z2;
  out.w2 = -mat.w2;
  out.x3 = -mat.x3;
  out.y3 = -mat.y3;
  out.z3 = -mat.z3;
  out.w3 = -mat.w3;
  out.x4 = -mat.x4;
  out.y4 = -mat.y4;
  out.z4 = -mat.z4;
  out.w4 = -mat.w4;
  return out;
}

template<typename T>
SHOGLE_MATH_DEF nummat<4, 4, T> operator-(const nummat<4, 4, T>& a,
                                          const nummat<4, 4, T>& b) noexcept {
  nummat<4, 4, T> out;
  out.x1 = a.x1 - b.x1;
  out.y1 = a.y1 - b.y1;
  out.z1 = a.z1 - b.z1;
  out.w1 = a.w1 - b.w1;
  out.x2 = a.x2 - b.x2;
  out.y2 = a.y2 - b.y2;
  out.z2 = a.z2 - b.z2;
  out.w2 = a.w2 - b.w2;
  out.x3 = a.x3 - b.x3;
  out.y3 = a.y3 - b.y3;
  out.z3 = a.z3 - b.z3;
  out.w3 = a.w3 - b.w3;
  out.x4 = a.x4 - b.x4;
  out.y4 = a.y4 - b.y4;
  out.z4 = a.z4 - b.z4;
  out.w4 = a.w4 - b.w4;
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<4, 4, T> operator-(const nummat<4, 4, T>& mat, U scalar) noexcept {
  nummat<4, 4, T> out;
  out.x1 = mat.x1 - static_cast<T>(scalar);
  out.y1 = mat.y1 - static_cast<T>(scalar);
  out.z1 = mat.z1 - static_cast<T>(scalar);
  out.w1 = mat.w1 - static_cast<T>(scalar);
  out.x2 = mat.x2 - static_cast<T>(scalar);
  out.y2 = mat.y2 - static_cast<T>(scalar);
  out.z2 = mat.z2 - static_cast<T>(scalar);
  out.w2 = mat.w2 - static_cast<T>(scalar);
  out.x3 = mat.x3 - static_cast<T>(scalar);
  out.y3 = mat.y3 - static_cast<T>(scalar);
  out.z3 = mat.z3 - static_cast<T>(scalar);
  out.w3 = mat.w3 - static_cast<T>(scalar);
  out.x4 = mat.x4 - static_cast<T>(scalar);
  out.y4 = mat.y4 - static_cast<T>(scalar);
  out.z4 = mat.z4 - static_cast<T>(scalar);
  out.w4 = mat.w4 - static_cast<T>(scalar);
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<4, 4, T> operator-(U scalar, const nummat<4, 4, T>& mat) noexcept {
  nummat<4, 4, T> out;
  out.x1 = static_cast<T>(scalar) - mat.x1;
  out.y1 = static_cast<T>(scalar) - mat.y1;
  out.z1 = static_cast<T>(scalar) - mat.z1;
  out.w1 = static_cast<T>(scalar) - mat.w1;
  out.x2 = static_cast<T>(scalar) - mat.x2;
  out.y2 = static_cast<T>(scalar) - mat.y2;
  out.z2 = static_cast<T>(scalar) - mat.z2;
  out.w2 = static_cast<T>(scalar) - mat.w2;
  out.x3 = static_cast<T>(scalar) - mat.x3;
  out.y3 = static_cast<T>(scalar) - mat.y3;
  out.z3 = static_cast<T>(scalar) - mat.z3;
  out.w3 = static_cast<T>(scalar) - mat.w3;
  out.x4 = static_cast<T>(scalar) - mat.x4;
  out.y4 = static_cast<T>(scalar) - mat.y4;
  out.z4 = static_cast<T>(scalar) - mat.z4;
  out.w4 = static_cast<T>(scalar) - mat.w4;
  return out;
}

template<typename T>
SHOGLE_MATH_DEF nummat<4, 4, T> operator*(const nummat<4, 4, T>& a,
                                          const nummat<4, 4, T>& b) noexcept {
  nummat<4, 4, T> out;
  out.x1 = (a.x1 * b.x1) + (a.x2 * b.y1) + (a.x3 * b.z1) + (a.x4 * b.w1);
  out.y1 = (a.y1 * b.x1) + (a.y2 * b.y1) + (a.y3 * b.z1) + (a.y4 * b.w1);
  out.z1 = (a.z1 * b.x1) + (a.z2 * b.y1) + (a.z3 * b.z1) + (a.z4 * b.w1);
  out.w1 = (a.w1 * b.x1) + (a.w2 * b.y1) + (a.w3 * b.z1) + (a.w4 * b.w1);
  out.x2 = (a.x1 * b.x2) + (a.x2 * b.y2) + (a.x3 * b.z2) + (a.x4 * b.w2);
  out.y2 = (a.y1 * b.x2) + (a.y2 * b.y2) + (a.y3 * b.z2) + (a.y4 * b.w2);
  out.z2 = (a.z1 * b.x2) + (a.z2 * b.y2) + (a.z3 * b.z2) + (a.z4 * b.w2);
  out.w2 = (a.w1 * b.x2) + (a.w2 * b.y2) + (a.w3 * b.z2) + (a.w4 * b.w2);
  out.x3 = (a.x1 * b.x3) + (a.x2 * b.y3) + (a.x3 * b.z3) + (a.x4 * b.w3);
  out.y3 = (a.y1 * b.x3) + (a.y2 * b.y3) + (a.y3 * b.z3) + (a.y4 * b.w3);
  out.z3 = (a.z1 * b.x3) + (a.z2 * b.y3) + (a.z3 * b.z3) + (a.z4 * b.w3);
  out.w3 = (a.w1 * b.x3) + (a.w2 * b.y3) + (a.w3 * b.z3) + (a.w4 * b.w3);
  out.x4 = (a.x1 * b.x4) + (a.x2 * b.y4) + (a.x3 * b.z4) + (a.x4 * b.w4);
  out.y4 = (a.y1 * b.x4) + (a.y2 * b.y4) + (a.y3 * b.z4) + (a.y4 * b.w4);
  out.z4 = (a.z1 * b.x4) + (a.z2 * b.y4) + (a.z3 * b.z4) + (a.z4 * b.w4);
  out.w4 = (a.w1 * b.x4) + (a.w2 * b.y4) + (a.w3 * b.z4) + (a.w4 * b.w4);
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<4, 4, T> operator*(const nummat<4, 4, T>& mat, U scalar) noexcept {
  nummat<4, 4, T> out;
  out.x1 = mat.x1 * static_cast<T>(scalar);
  out.y1 = mat.y1 * static_cast<T>(scalar);
  out.z1 = mat.z1 * static_cast<T>(scalar);
  out.w1 = mat.w1 * static_cast<T>(scalar);
  out.x2 = mat.x2 * static_cast<T>(scalar);
  out.y2 = mat.y2 * static_cast<T>(scalar);
  out.z2 = mat.z2 * static_cast<T>(scalar);
  out.w2 = mat.w2 * static_cast<T>(scalar);
  out.x3 = mat.x3 * static_cast<T>(scalar);
  out.y3 = mat.y3 * static_cast<T>(scalar);
  out.z3 = mat.z3 * static_cast<T>(scalar);
  out.w3 = mat.w3 * static_cast<T>(scalar);
  out.x4 = mat.x4 * static_cast<T>(scalar);
  out.y4 = mat.y4 * static_cast<T>(scalar);
  out.z4 = mat.z4 * static_cast<T>(scalar);
  out.w4 = mat.w4 * static_cast<T>(scalar);
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<4, 4, T> operator*(U scalar, const nummat<4, 4, T>& mat) noexcept {
  nummat<4, 4, T> out;
  out.x1 = static_cast<T>(scalar) * mat.x1;
  out.y1 = static_cast<T>(scalar) * mat.y1;
  out.z1 = static_cast<T>(scalar) * mat.z1;
  out.w1 = static_cast<T>(scalar) * mat.w1;
  out.x2 = static_cast<T>(scalar) * mat.x2;
  out.y2 = static_cast<T>(scalar) * mat.y2;
  out.z2 = static_cast<T>(scalar) * mat.z2;
  out.w2 = static_cast<T>(scalar) * mat.w2;
  out.x3 = static_cast<T>(scalar) * mat.x3;
  out.y3 = static_cast<T>(scalar) * mat.y3;
  out.z3 = static_cast<T>(scalar) * mat.z3;
  out.w3 = static_cast<T>(scalar) * mat.w3;
  out.x4 = static_cast<T>(scalar) * mat.x4;
  out.y4 = static_cast<T>(scalar) * mat.y4;
  out.z4 = static_cast<T>(scalar) * mat.z4;
  out.w4 = static_cast<T>(scalar) * mat.w4;
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF typename nummat<4, 4, T>::col_type operator*(const nummat<4, 4, T>& m,
                                                             const numvec<4, U>& v) noexcept {
  typename nummat<4, 4, T>::col_type out;
  out.x = m.x1 * v.x + m.x2 * v.y + m.x3 * v.z + m.x4 * v.w;
  out.y = m.y1 * v.x + m.y2 * v.y + m.y3 * v.z + m.y4 * v.w;
  out.z = m.z1 * v.x + m.z2 * v.y + m.z3 * v.z + m.z4 * v.w;
  out.w = m.w1 * v.x + m.w2 * v.y + m.w3 * v.z + m.w4 * v.w;
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF typename nummat<4, 4, T>::row_type operator*(const numvec<4, U>& v,
                                                             const nummat<4, 4, T>& m) noexcept {
  typename nummat<4, 4, T>::row_type out;
  out.x = v.x * m.x1 + v.y * m.y1 + v.z * m.z1 + v.w * m.w1;
  out.y = v.x * m.x2 + v.y * m.y2 + v.z * m.z2 + v.w * m.w2;
  out.z = v.x * m.x3 + v.y * m.y3 + v.z * m.z3 + v.w * m.w3;
  out.w = v.x * m.x4 + v.y * m.y4 + v.z * m.z4 + v.w * m.w4;
  return out;
}

template<typename T>
SHOGLE_MATH_DEF nummat<4, 4, T> operator/(const nummat<4, 4, T>& a,
                                          const nummat<4, 4, T>& b) noexcept {
  return a * ::shogle::math::inverse(b);
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<4, 4, T> operator/(const nummat<4, 4, T>& mat, U scalar) noexcept {
  nummat<4, 4, T> out;
  out.x1 = mat.x1 / static_cast<T>(scalar);
  out.y1 = mat.y1 / static_cast<T>(scalar);
  out.z1 = mat.z1 / static_cast<T>(scalar);
  out.w1 = mat.w1 / static_cast<T>(scalar);
  out.x2 = mat.x2 / static_cast<T>(scalar);
  out.y2 = mat.y2 / static_cast<T>(scalar);
  out.z2 = mat.z2 / static_cast<T>(scalar);
  out.w2 = mat.w2 / static_cast<T>(scalar);
  out.x3 = mat.x3 / static_cast<T>(scalar);
  out.y3 = mat.y3 / static_cast<T>(scalar);
  out.z3 = mat.z3 / static_cast<T>(scalar);
  out.w3 = mat.w3 / static_cast<T>(scalar);
  out.x4 = mat.x4 / static_cast<T>(scalar);
  out.y4 = mat.y4 / static_cast<T>(scalar);
  out.z4 = mat.z4 / static_cast<T>(scalar);
  out.w4 = mat.w4 / static_cast<T>(scalar);
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF nummat<4, 4, T> operator/(U scalar, const nummat<4, 4, T>& mat) noexcept {
  nummat<4, 4, T> out;
  out.x1 = static_cast<T>(scalar) / mat.x1;
  out.y1 = static_cast<T>(scalar) / mat.y1;
  out.z1 = static_cast<T>(scalar) / mat.z1;
  out.w1 = static_cast<T>(scalar) / mat.w1;
  out.x2 = static_cast<T>(scalar) / mat.x2;
  out.y2 = static_cast<T>(scalar) / mat.y2;
  out.z2 = static_cast<T>(scalar) / mat.z2;
  out.w2 = static_cast<T>(scalar) / mat.w2;
  out.x3 = static_cast<T>(scalar) / mat.x3;
  out.y3 = static_cast<T>(scalar) / mat.y3;
  out.z3 = static_cast<T>(scalar) / mat.z3;
  out.w3 = static_cast<T>(scalar) / mat.w3;
  out.x4 = static_cast<T>(scalar) / mat.x4;
  out.y4 = static_cast<T>(scalar) / mat.y4;
  out.z4 = static_cast<T>(scalar) / mat.z4;
  out.w4 = static_cast<T>(scalar) / mat.w4;
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF typename nummat<4, 4, T>::col_type operator/(const nummat<4, 4, T>& mat,
                                                             const numvec<4, U>& vec) noexcept {
  return ::shogle::math::inverse(mat) * vec;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF typename nummat<4, 4, T>::row_type operator/(const numvec<4, U>& vec,
                                                             const nummat<4, 4, T>& mat) noexcept {
  return vec * ::shogle::math::inverse(mat);
}

} // namespace shogle
