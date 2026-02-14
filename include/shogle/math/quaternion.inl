#define SHOGLE_MATH_QUATERNION_INL
#include "./quaternion.hpp"
#undef SHOGLE_MATH_QUATERNION_INL

namespace shogle {

template<typename U, typename T>
requires(math::numeric_convertible<U, T>)
SHOGLE_MATH_DEF numquat<U> vec_cast(const numquat<T>& q) noexcept {
  return {static_cast<U>(q.w), static_cast<U>(q.x), static_cast<U>(q.y), static_cast<U>(q.z)};
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T>& numquat<T>::operator=(U scalar) noexcept {
  this->w = static_cast<T>(scalar);
  this->x = static_cast<T>(scalar);
  this->y = static_cast<T>(scalar);
  this->z = static_cast<T>(scalar);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T>& numquat<T>::operator+=(U scalar) noexcept {
  this->w += static_cast<T>(scalar);
  this->x += static_cast<T>(scalar);
  this->y += static_cast<T>(scalar);
  this->z += static_cast<T>(scalar);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T>& numquat<T>::operator-=(U scalar) noexcept {
  this->w -= static_cast<T>(scalar);
  this->x -= static_cast<T>(scalar);
  this->y -= static_cast<T>(scalar);
  this->z -= static_cast<T>(scalar);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T>& numquat<T>::operator*=(U scalar) noexcept {
  this->w *= static_cast<T>(scalar);
  this->x *= static_cast<T>(scalar);
  this->y *= static_cast<T>(scalar);
  this->z *= static_cast<T>(scalar);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T>& numquat<T>::operator/=(U scalar) noexcept {
  this->w /= static_cast<T>(scalar);
  this->x /= static_cast<T>(scalar);
  this->y /= static_cast<T>(scalar);
  this->z /= static_cast<T>(scalar);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T>& numquat<T>::operator=(const numquat<U>& other) noexcept {
  this->w = static_cast<T>(other.w);
  this->x = static_cast<T>(other.x);
  this->y = static_cast<T>(other.y);
  this->z = static_cast<T>(other.z);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T>& numquat<T>::operator+=(const numquat<U>& other) noexcept {
  this->w += static_cast<T>(other.w);
  this->x += static_cast<T>(other.x);
  this->y += static_cast<T>(other.y);
  this->z += static_cast<T>(other.z);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T>& numquat<T>::operator-=(const numquat<U>& other) noexcept {
  this->w -= static_cast<T>(other.w);
  this->x -= static_cast<T>(other.x);
  this->y -= static_cast<T>(other.y);
  this->z -= static_cast<T>(other.z);
  return *this;
}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T>& numquat<T>::operator*=(const numquat<U>& other) noexcept {
  const auto q = ::shogle::vec_cast<T>(other);
  const numquat<T> self = *this;
  this->w = (self.w * q.w) - (self.x * q.x) - (self.y * q.y) - (self.z * q.z);
  this->x = (self.w * q.x) + (self.x * q.w) + (self.y * q.z) - (self.z * q.y);
  this->y = (self.w * q.y) + (self.y * q.w) + (self.z * q.x) - (self.x * q.z);
  this->z = (self.w * q.z) + (self.z * q.w) + (self.x * q.y) - (self.y * q.x);
  return *this;
}

template<typename T>
SHOGLE_MATH_DEF bool operator==(const numquat<T>& a, const numquat<T>& b) noexcept {
  return (a.w == b.w) && (a.x == b.x) && (a.y == b.y) && (a.z == b.z);
}

template<typename T>
SHOGLE_MATH_DEF bool operator!=(const numquat<T>& a, const numquat<T>& b) noexcept {
  return (a.w != b.w) || (a.x != b.x) || (a.y != b.y) || (a.z != b.z);
}

template<typename T>
SHOGLE_MATH_DEF numquat<T> operator+(const numquat<T>& q) noexcept {
  numquat<T> out;
  out.w = +q.w;
  out.x = +q.x;
  out.y = +q.y;
  out.z = +q.z;
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T> operator+(const numquat<T>& a, const numquat<U>& b) noexcept {
  numquat<T> out;
  out.w = a.w + static_cast<T>(b.w);
  out.x = a.x + static_cast<T>(b.x);
  out.y = a.y + static_cast<T>(b.y);
  out.z = a.z + static_cast<T>(b.z);
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T> operator+(const numquat<T>& q, U scalar) noexcept {
  numquat<T> out;
  out.w = q.w + static_cast<T>(scalar);
  out.x = q.x + static_cast<T>(scalar);
  out.y = q.y + static_cast<T>(scalar);
  out.z = q.z + static_cast<T>(scalar);
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T> operator+(U scalar, const numquat<T>& q) noexcept {
  numquat<T> out;
  out.w = static_cast<T>(scalar) + q.w;
  out.x = static_cast<T>(scalar) + q.x;
  out.y = static_cast<T>(scalar) + q.y;
  out.z = static_cast<T>(scalar) + q.z;
  return out;
}

template<typename T>
SHOGLE_MATH_DEF numquat<T> operator-(const numquat<T>& q) noexcept {
  numquat<T> out;
  out.w = -q.w;
  out.x = -q.x;
  out.y = -q.y;
  out.z = -q.z;
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T> operator-(const numquat<T>& a, const numquat<U>& b) noexcept {
  numquat<T> out;
  out.w = a.w - static_cast<T>(b.w);
  out.x = a.x - static_cast<T>(b.x);
  out.y = a.y - static_cast<T>(b.y);
  out.z = a.z - static_cast<T>(b.z);
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T> operator-(const numquat<T>& q, U scalar) noexcept {
  numquat<T> out;
  out.w = q.w - static_cast<T>(scalar);
  out.x = q.x - static_cast<T>(scalar);
  out.y = q.y - static_cast<T>(scalar);
  out.z = q.z - static_cast<T>(scalar);
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T> operator-(U scalar, const numquat<T>& q) noexcept {
  numquat<T> out;
  out.w = static_cast<T>(scalar) - q.w;
  out.x = static_cast<T>(scalar) - q.x;
  out.y = static_cast<T>(scalar) - q.y;
  out.z = static_cast<T>(scalar) - q.z;
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T> operator*(const numquat<T>& a, const numquat<U>& b) noexcept {
  const auto q = ::shogle::vec_cast<T>(b);
  numquat<T> out;
  out.w = (a.w * q.w) - (a.x * q.x) - (a.y * q.y) - (a.z * q.z);
  out.x = (a.w * q.x) + (a.x * q.w) + (a.y * q.z) - (a.z * q.y);
  out.y = (a.w * q.y) + (a.y * q.w) + (a.z * q.x) - (a.x * q.z);
  out.z = (a.w * q.z) + (a.z * q.w) + (a.x * q.y) - (a.y * q.x);
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numvec<3, T> operator*(const numquat<T>& q, const numvec<3, U>& v) noexcept {
  const auto vt = ::shogle::vec_cast<T>(v);
  const numvec<3, T> qv = ::shogle::math::to_vec3(q);
  const numvec<3, T> c1 = ::shogle::math::cross(qv, vt);
  const numvec<3, T> c2 = ::shogle::math::cross(qv, c1);
  return vt + (((c1 * q.w) + c2) * T(2));
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numvec<4, T> operator*(const numquat<T>& q, const numvec<4, U>& v) noexcept {
  const numvec<3, U> v3{v.x, v.y, v.z};
  const numvec<3, T> vq = q * ::shogle::vec_cast<T>(v3);
  return numvec<4, T>{vq.x, vq.y, vq.z, static_cast<T>(v.w)};
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T> operator*(const numquat<T>& q, U scalar) noexcept {
  numquat<T> out;
  out.w = q.w * static_cast<T>(scalar);
  out.x = q.x * static_cast<T>(scalar);
  out.y = q.y * static_cast<T>(scalar);
  out.z = q.z * static_cast<T>(scalar);
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T> operator*(U scalar, const numquat<T>& q) noexcept {
  numquat<T> out;
  out.w = static_cast<T>(scalar) * q.w;
  out.x = static_cast<T>(scalar) * q.x;
  out.y = static_cast<T>(scalar) * q.y;
  out.z = static_cast<T>(scalar) * q.z;
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T> operator/(const numquat<T>& q, U scalar) noexcept {
  numquat<T> out;
  out.w = q.w / static_cast<T>(scalar);
  out.x = q.x / static_cast<T>(scalar);
  out.y = q.y / static_cast<T>(scalar);
  out.z = q.z / static_cast<T>(scalar);
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T> operator/(U scalar, const numquat<T>& q) noexcept {
  numquat<T> out;
  out.w = static_cast<T>(scalar) / q.w;
  out.x = static_cast<T>(scalar) / q.x;
  out.y = static_cast<T>(scalar) / q.y;
  out.z = static_cast<T>(scalar) / q.z;
  return out;
}

} // namespace shogle

namespace shogle::math {

template<typename T>
SHOGLE_MATH_DEF numquat<T> conjugate(const numquat<T>& q) noexcept {
  numquat<T> out;
  out.w = q.w;
  out.x = -q.x;
  out.y = -q.y;
  out.z = -q.z;
  return out;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF T dot(const numquat<T>& a, const numquat<U>& b) noexcept {
  const T w = a.w * static_cast<T>(b.w);
  const T x = a.x * static_cast<T>(b.x);
  const T y = a.y * static_cast<T>(b.y);
  const T z = a.z * static_cast<T>(b.z);
  return (w + x + y + z);
}

template<typename T>
SHOGLE_MATH_DEF numquat<T> inverse(const numquat<T>& q) noexcept {
  const T d = ::shogle::math::dot(q, q);
  numquat<T> out;
  out.w = q.w / d;
  out.x = -q.x / d;
  out.y = -q.y / d;
  out.z = -q.z / d;
  return out;
}

template<typename T>
SHOGLE_MATH_DEF nummat<3, 3, T> to_mat3(const numquat<T>& q) noexcept {
  const T qxx = q.x * q.x;
  const T qyy = q.y * q.y;
  const T qzz = q.z * q.z;
  const T qxz = q.x * q.z;
  const T qxy = q.x * q.y;
  const T qyz = q.y * q.z;
  const T qwx = q.w * q.x;
  const T qwy = q.w * q.y;
  const T qwz = q.w * q.z;

  nummat<3, 3, T> out;
  out.x1 = T(1) - (T(2) * (qyy + qzz));
  out.y1 = T(2) * (qxy + qwz);
  out.z1 = T(2) * (qxz - qwy);
  out.x2 = T(2) * (qxy - qwz);
  out.y2 = T(1) - (T(2) * (qxx + qzz));
  out.z2 = T(2) * (qyz + qwx);
  out.x3 = T(2) * (qxz + qwy);
  out.y3 = T(2) * (qyz - qwx);
  out.z3 = T(1) - (T(2) * (qxx + qyy));
  return out;
}

template<typename T>
SHOGLE_MATH_DEF nummat<4, 4, T> to_mat4(const numquat<T>& q) noexcept {
  const T qxx = q.x * q.x;
  const T qyy = q.y * q.y;
  const T qzz = q.z * q.z;
  const T qxz = q.x * q.z;
  const T qxy = q.x * q.y;
  const T qyz = q.y * q.z;
  const T qwx = q.w * q.x;
  const T qwy = q.w * q.y;
  const T qwz = q.w * q.z;

  nummat<4, 4, T> out;
  out.x1 = T(1) - (T(2) * (qyy + qzz));
  out.y1 = T(2) * (qxy + qwz);
  out.z1 = T(2) * (qxz - qwy);
  out.w1 = T(0);
  out.x2 = T(2) * (qxy - qwz);
  out.y2 = T(1) - (T(2) * (qxx + qzz));
  out.z2 = T(2) * (qyz + qwx);
  out.w2 = T(0);
  out.x3 = T(2) * (qxz + qwy);
  out.y3 = T(2) * (qyz - qwx);
  out.z3 = T(1) - (T(2) * (qxx + qyy));
  out.w3 = T(0);
  out.x4 = T(0);
  out.y4 = T(0);
  out.z4 = T(0);
  out.w4 = T(1);
  return out;
}

template<typename T>
SHOGLE_MATH_DEF numquat<T> to_quat(const nummat<3, 3, T>& m) noexcept {
  const auto [biggest, idx] = [](auto&& m) -> std::pair<T, u32> {
    const T x2m1 = m.x1 - m.x2 - m.x3;
    const T y2m1 = m.y2 - m.x1 - m.z3;
    const T z2m1 = m.z3 - m.x1 - m.y2;
    const T w2m1 = m.x1 + m.y2 + m.z3;
    u32 idx = 0;
    T biggest = w2m1;
    if (x2m1 > biggest) {
      biggest = x2m1;
      idx = 1;
    }
    if (y2m1 > biggest) {
      biggest = y2m1;
      idx = 2;
    }
    if (z2m1 > biggest) {
      biggest = z2m1;
      idx = 3;
    }
    return {::shogle::math::sqrt(biggest + T(1)) * T(0.5), idx};
  }(m);

  const T mult = T(0.25) / biggest;
  const T y3z2m = (m.y3 - m.z2) * mult;
  const T z1x3m = (m.z1 - m.x3) * mult;
  const T y2x1m = (m.y2 - m.x1) * mult;
  const T y2x2p = (m.y2 + m.x1) * mult;
  const T z1x3p = (m.z1 + m.x3) * mult;
  const T y3z2p = (m.y3 + m.z2) * mult;
  numquat<T> out[] = {
    {biggest, y3z2m, z1x3m, y2x1m},
    {y3z2m, biggest, y2x2p, z1x3p},
    {z1x3m, y2x2p, biggest, y3z2p},
    {y2x1m, z1x3p, y3z2p, biggest},
  };
  return out[idx];
}

template<typename T>
SHOGLE_MATH_DEF numquat<T> to_quat(const nummat<4, 4, T>& m) noexcept {
  return ::shogle::math::to_quat(::shogle::math::to_mat3(m));
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T> to_quat(T angle, const numvec<3, U>& axis) noexcept {
  const numvec<3, U> norm = ::shogle::math::normalize(axis);
  numquat<T> out;
  out.w = ::shogle::math::cos(angle * T(0.5));
  out.x = ::shogle::math::sin(angle * T(0.5)) * axis.x;
  out.y = ::shogle::math::sin(angle * T(0.5)) * axis.y;
  out.z = ::shogle::math::sin(angle * T(0.5)) * axis.z;
  return out;
}

template<typename T>
SHOGLE_MATH_DEF numquat<T> to_quat(const numvec<3, T>& euler) noexcept {
  const T cx = ::shogle::math::cos(euler.x * T(0.5));
  const T cy = ::shogle::math::cos(euler.y * T(0.5));
  const T cz = ::shogle::math::cos(euler.z * T(0.5));
  const T sx = ::shogle::math::sin(euler.x * T(0.5));
  const T sy = ::shogle::math::sin(euler.y * T(0.5));
  const T sz = ::shogle::math::sin(euler.z * T(0.5));

  numquat<T> out;
  out.w = (cx * cy * cz) + (sx * sy * sz);
  out.x = (sx * cy * cz) - (cx * sy * sz);
  out.y = (cx * sy * cz) + (sx * cy * sz);
  out.z = (cx * cy * sz) - (sx * sy * cz);
  return out;
}

template<typename T>
SHOGLE_MATH_DEF T roll(const numquat<T>& q) noexcept {
  const T y = T(2) * ((q.x * q.y) + (q.w * q.z));
  const T x = (q.w * q.w) + (q.x * q.x) - (q.y * q.y) - (q.z * q.z);
  return ::shogle::math::atan2(y, x);
}

template<typename T>
SHOGLE_MATH_DEF T pitch(const numquat<T>& q) noexcept {
  const T y = T(2) * ((q.y * q.z) + (q.w * q.x));
  const T x = (q.w * q.w) - (q.x * q.x) - (q.y * q.y) + (q.z * q.z);

  const bool singularity = [](auto&& x, auto&& y) -> bool {
    if constexpr (std::floating_point<T>) {
      return ::shogle::math::fequal(x, T(0)) && ::shogle::math::fequal(y, T(0));
    } else {
      return (x == T(0)) && (y == T(0));
    }
  }(x, y);

  return singularity ? (T(2) * ::shogle::math::atan2(q.x, q.w)) : ::shogle::math::atan2(y, x);
}

template<typename T>
SHOGLE_MATH_DEF T yaw(const numquat<T>& q) noexcept {
  const T v = T(-2) * ((q.x * q.z) - (q.w * q.y));
  return ::shogle::math::asin(::shogle::math::clamp(v, T(-1), T(1)));
}

template<typename T>
SHOGLE_MATH_DEF numvec<3, T> to_euler(const numquat<T>& q) noexcept {
  numvec<3, T> out;
  out.x = ::shogle::math::pitch(q);
  out.y = ::shogle::math::yaw(q);
  out.z = ::shogle::math::roll(q);
  return out;
}

} // namespace shogle::math

namespace shogle {

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T>::numquat(const numvec<3, U>& euler) noexcept :
    numquat(::shogle::math::to_quat(euler)) {}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T>::numquat(const nummat<3, 3, U>& m) noexcept :
    numquat(::shogle::math::to_quat(m)) {}

template<math::numeric_type T>
template<math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numquat<T>::numquat(const nummat<4, 4, U>& m) noexcept :
    numquat(::shogle::math::to_quat(m)) {}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numvec<3, T> operator*(const numvec<3, U>& v, const numquat<T>& q) noexcept {
  return ::shogle::math::inverse(q) * v;
}

template<typename T, math::numeric_convertible<T> U>
SHOGLE_MATH_DEF numvec<4, T> operator*(const numvec<4, U>& v, const numquat<T>& q) noexcept {
  return ::shogle::math::inverse(q) * v;
}

} // namespace shogle
