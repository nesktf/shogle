#pragma once

#include "./matrix.hpp"

#if SHOGLE_USE_GLM
#include <glm/gtc/quaternion.hpp>
#endif

namespace ntf {

#if SHOGLE_USE_GLM
template<typename T>
using qua = glm::qua<T>;
#endif

using quat = qua<float32>;
using dquat = qua<float64>;

template<typename T>
constexpr qua<T> axisquat(const T& ang, const vec<3, T>& axis) {
  return qua<T>{glm::cos(ang*T{0.5}), glm::sin(ang*T{0.5})*axis};
}

template<typename T>
constexpr qua<T> eulerquat(const vec<3, T>& rot) {
  // return
  //   axisquat(rot.x, vec<3, T>{T{1}, T{0}, T{0}}) *
  //   axisquat(rot.y, vec<3, T>{T{0}, T{1}, T{0}}) *
  //   axisquat(rot.z, vec<3, T>{T{0}, T{0}, T{1}});

  const T cr = glm::cos(rot.x*T{0.5});
  const T sr = glm::sin(rot.x*T{0.5});
  const T cp = glm::cos(rot.y*T{0.5});
  const T sp = glm::sin(rot.y*T{0.5});
  const T cy = glm::cos(rot.z*T{0.5});
  const T sy = glm::cos(rot.z*T{0.5});

  qua<T> q;

  q.w = cr * cp * cy + sr * sp * sy;
  q.x = sr * cp * cy - cr * sp * sy;
  q.y = cr * sp * cy + sr * cp * sy;
  q.z = cr * cp * sy - sr * sp * cy;

  return q;
}

template<typename T>
constexpr vec<3, T> eulerquat(const qua<T>& q) noexcept {
  vec<3, T> euler;

  // Roll
  const T sinr_cosp = T{2}*(q.w*q.x + q.y*q.z);
  const T cosr_cosp = T{1} - T{2}*(q.x*q.x + q.y*q.y);
  euler.x = std::atan2(sinr_cosp, cosr_cosp);

  // Pitch
  const T sinp = std::sqrt(T{1} + T{2}*(q.w*q.y - q.x*q.z));
  const T cosp = std::sqrt(T{1} - T{2}*(q.w*q.y - q.x*q.z));
  euler.y = T{2}*std::atan2(sinp, cosp) - glm::pi<T>()/T{2};

  // Yaw
  const T siny_cosp = T{2}*(q.w*q.z + q.x*q.y);
  const T cosy_cosp = T{1} - T{2}*(q.y*q.y + q.z*q.z);
  euler.z = std::atan2(siny_cosp, cosy_cosp);

  return euler;
}

template<typename T>
mat<4, 4, T> build_trs_matrix(const vec<3, T>& pos,
                              const vec<3, T>& scale,
                              const vec<3, T>& pivot,
                              const qua<T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, pos);
  m = glm::translate(m, pivot);
  m*= glm::mat4_cast(rot);
  m = glm::scale(m, scale);
  m = glm::translate(m, -pivot);
  return m;
}

template<typename T>
mat<4, 4, T> build_trs_matrix(const vec<2, T>& pos,
                              const vec<2, T>& scale,
                              const vec<2, T>& pivot,
                              const qua<T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, vec<3, T>{pos, T{0}});
  m = glm::translate(m, vec<3, T>{pivot.x, pivot.y, T{0}});
  m*= glm::mat4_cast(rot);
  m = glm::scale(m, scale);
  m = glm::translate(m, vec<3, T>{-pivot.x, -pivot.y, T{0}});
  return m;
}

} // namespace ntf
