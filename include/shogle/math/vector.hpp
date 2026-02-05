#pragma once

#include <shogle/math/common.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace shogle::math {

template<u32 N, typename T>
using vec = glm::vec<N, T>;

using vec2 = ::shogle::math::vec<2, f32>;
using vec3 = ::shogle::math::vec<3, f32>;
using vec4 = ::shogle::math::vec<4, f32>;

using dvec2 = ::shogle::math::vec<2, f64>;
using dvec3 = ::shogle::math::vec<3, f64>;
using dvec4 = ::shogle::math::vec<4, f64>;

using ivec2 = ::shogle::math::vec<2, i32>;
using ivec3 = ::shogle::math::vec<3, i32>;
using ivec4 = ::shogle::math::vec<4, i32>;

using uvec2 = ::shogle::math::vec<2, u32>;
using uvec3 = ::shogle::math::vec<3, u32>;
using uvec4 = ::shogle::math::vec<4, u32>;

template<typename T>
constexpr auto vec_ptr(T& vec) {
  return glm::value_ptr(vec);
}

constexpr inline f32 norm2(vec2 v) {
  return v.x * v.x + v.y * v.y;
}

constexpr inline f32 norm(vec2 v) {
  return glm::sqrt(norm2(v));
}

constexpr inline vec2 normalize(vec2 v) {
  return v / glm::sqrt(norm2(v));
}

constexpr inline vec3 carcoord(vec3 v) {
  return {v.z, v.x, v.y};
}

constexpr inline vec3 glcoord(vec3 v) {
  return {v.y, v.z, v.x};
}

constexpr inline vec3 carcoord(f32 rho, f32 theta, f32 phi) {
  return rho *
         vec3{glm::sin(theta) * glm::cos(phi), glm::sin(theta) * glm::sin(phi), glm::cos(theta)};
}

constexpr inline vec3 glcoord(f32 rho, f32 theta, f32 phi) {
  return glcoord(carcoord(rho, theta, phi));
}

constexpr inline vec2 expiv(f32 theta) {
  return vec2{glm::cos(theta), glm::sin(theta)};
}

template<typename T>
concept vertex_type = (ntf::meta::same_as_any<T, vec2, vec3, vec4>);

template<u32 N, u32 M, typename T>
using mat = glm::mat<N, M, T>;

using mat3 = mat<3, 3, f32>;
using mat4 = mat<4, 4, f32>;

using dmat3 = mat<3, 3, f64>;
using dmat4 = mat<4, 4, f64>;

template<typename T>
mat<4, 4, T> build_trs_matrix(const ::shogle::math::vec<2, T>& pos,
                              const ::shogle::math::vec<2, T>& scale,
                              const ::shogle::math::vec<3, T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, ::shogle::math::vec<3, T>{pos, T{0}});
  m = glm::rotate(m, rot.z, ::shogle::math::vec<3, T>{T{0}, T{0}, T{1}});
  m = glm::rotate(m, rot.y, ::shogle::math::vec<3, T>{T{0}, T{1}, T{0}});
  m = glm::rotate(m, rot.x, ::shogle::math::vec<3, T>{T{1}, T{0}, T{0}});
  m = glm::scale(m, ::shogle::math::vec<3, T>{scale, T{1}});
  return m;
}

template<typename T>
mat<4, 4, T> build_trs_matrix(const ::shogle::math::vec<3, T>& pos,
                              const ::shogle::math::vec<3, T>& scale,
                              const ::shogle::math::vec<3, T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, pos);
  m = glm::rotate(m, rot.z, ::shogle::math::vec<3, T>{T{0}, T{0}, T{1}});
  m = glm::rotate(m, rot.y, ::shogle::math::vec<3, T>{T{0}, T{1}, T{0}});
  m = glm::rotate(m, rot.x, ::shogle::math::vec<3, T>{T{1}, T{0}, T{0}});
  m = glm::scale(m, scale);
  return m;
}

template<typename T>
mat<4, 4, T> build_trs_matrix(const ::shogle::math::vec<2, T>& pos,
                              const ::shogle::math::vec<2, T>& scale,
                              const ::shogle::math::vec<2, T>& pivot,
                              const ::shogle::math::vec<3, T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, ::shogle::math::vec<3, T>{pos.x + pivot.x, pos.y + pivot.y, T{0}});
  m = glm::rotate(m, rot.z, ::shogle::math::vec<3, T>{T{0}, T{0}, T{1}});
  m = glm::rotate(m, rot.y, ::shogle::math::vec<3, T>{T{0}, T{1}, T{0}});
  m = glm::rotate(m, rot.x, ::shogle::math::vec<3, T>{T{1}, T{0}, T{0}});
  m = glm::translate(m, ::shogle::math::vec<3, T>{-pivot.x, -pivot.y, T{0}});
  m = glm::scale(m, ::shogle::math::vec<3, T>{scale.x, scale.y, T{1}});
  return m;
}

template<typename T>
mat<4, 4, T> build_trs_matrix(const ::shogle::math::vec<3, T>& pos,
                              const ::shogle::math::vec<3, T>& scale,
                              const ::shogle::math::vec<3, T>& pivot,
                              const ::shogle::math::vec<3, T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, pos + pivot);
  m = glm::rotate(m, rot.z, ::shogle::math::vec<3, T>{T{0}, T{0}, T{1}});
  m = glm::rotate(m, rot.y, ::shogle::math::vec<3, T>{T{0}, T{1}, T{0}});
  m = glm::rotate(m, rot.x, ::shogle::math::vec<3, T>{T{1}, T{0}, T{0}});
  m = glm::translate(m, -pivot);
  m = glm::scale(m, scale);
  return m;
}

template<typename T>
mat<4, 4, T>
build_trs_matrix(const ::shogle::math::vec<2, T>& pos, const ::shogle::math::vec<2, T>& scale,
                 const ::shogle::math::vec<2, T>& pivot, const ::shogle::math::vec<2, T>& offset,
                 const ::shogle::math::vec<3, T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, ::shogle::math::vec<3, T>{pos.x + pivot.x, pos.y + pivot.y, T{0}});
  m = glm::rotate(m, rot.z, ::shogle::math::vec<3, T>{T{0}, T{0}, T{1}});
  m = glm::rotate(m, rot.y, ::shogle::math::vec<3, T>{T{0}, T{1}, T{0}});
  m = glm::rotate(m, rot.x, ::shogle::math::vec<3, T>{T{1}, T{0}, T{0}});
  m = glm::translate(m, ::shogle::math::vec<3, T>{offset.x - pivot.x, offset.x - pivot.y, T{0}});
  m = glm::scale(m, ::shogle::math::vec<3, T>{scale.x, scale.y, T{1}});
  m = glm::translate(m, ::shogle::math::vec<3, T>{-offset.x, -offset.y, T{0}});
  return m;
}

template<typename T>
mat<4, 4, T>
build_trs_matrix(const ::shogle::math::vec<3, T>& pos, const ::shogle::math::vec<3, T>& scale,
                 const ::shogle::math::vec<3, T>& pivot, const ::shogle::math::vec<3, T>& offset,
                 const ::shogle::math::vec<3, T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, pos + pivot);
  m = glm::rotate(m, rot.z, ::shogle::math::vec<3, T>{T{0}, T{0}, T{1}});
  m = glm::rotate(m, rot.y, ::shogle::math::vec<3, T>{T{0}, T{1}, T{0}});
  m = glm::rotate(m, rot.x, ::shogle::math::vec<3, T>{T{1}, T{0}, T{0}});
  m = glm::translate(m, offset - pivot);
  m = glm::scale(m, scale);
  return m;
}

template<typename T>
mat<4, 4, T> build_view_matrix(const ::shogle::math::vec<2, T>& pos,
                               const ::shogle::math::vec<2, T>& origin,
                               const ::shogle::math::vec<2, T>& zoom,
                               const ::shogle::math::vec<3, T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, ::shogle::math::vec<3, T>{origin.x, origin.y, T{0}});
  m = glm::rotate(m, rot.z, ::shogle::math::vec<3, T>{T{0}, T{0}, T{1}});
  m = glm::rotate(m, rot.y, ::shogle::math::vec<3, T>{T{0}, T{1}, T{0}});
  m = glm::rotate(m, rot.x, ::shogle::math::vec<3, T>{T{1}, T{0}, T{0}});
  m = glm::scale(m, ::shogle::math::vec<3, T>{zoom.x, zoom.y, T{1}});
  m = glm::translate(m, ::shogle::math::vec<3, T>{-pos.x, -pos.y, T{0}});
  return m;
}

template<typename T>
mat<4, 4, T> build_view_matrix(const ::shogle::math::vec<2, T>& pos,
                               const ::shogle::math::vec<2, T>& origin,
                               const ::shogle::math::vec<2, T>& zoom, const T& rot,
                               const ::shogle::math::vec<3, T>& axis) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, ::shogle::math::vec<3, T>{origin.x, origin.y, T{0}});
  m = glm::rotate(m, rot, axis);
  m = glm::scale(m, ::shogle::math::vec<3, T>{zoom.x, zoom.y, T{1}});
  m = glm::translate(m, ::shogle::math::vec<3, T>{-pos.x, -pos.y, T{0}});
  return m;
}

template<typename T>
mat<4, 4, T> build_view_matrix(const ::shogle::math::vec<3, T>& pos,
                               const ::shogle::math::vec<3, T>& dir,
                               const ::shogle::math::vec<3, T>& up) noexcept {
  return glm::lookAt(pos, pos + dir, up);
}

template<typename T>
mat<4, 4, T> build_proj_matrix(const ::shogle::math::vec<2, T>& viewport, const T& znear,
                               const T& zfar) noexcept {
  return glm::ortho(T{0}, viewport.x, viewport.y, T{0}, znear, zfar);
}

template<typename T>
mat<4, 4, T> build_proj_matrix(const ::shogle::math::vec<2, T>& viewport, const T& znear,
                               const T& zfar, const T& fov) noexcept {
  return glm::perspective(fov, viewport.x / viewport.y, znear, zfar);
}

template<typename T>
using complex = std::complex<T>;

using cmplx = complex<f32>;
using dcmplx = complex<f64>;
using icmplx = complex<i32>;
using ucmplx = complex<u32>;

constexpr inline f32 norm2(cmplx z) {
  return z.real() * z.real() + z.imag() * z.imag();
}

constexpr inline cmplx conv(const vec2& v) {
  return cmplx{v.x, v.y};
}

constexpr inline vec2 conv(const cmplx& z) {
  return vec2{z.real(), z.imag()};
}

constexpr inline cmplx normalize(cmplx z) {
  return z / glm::sqrt(norm2(z));
}

constexpr inline f32 norm(cmplx z) {
  return glm::sqrt(norm2(z));
}

constexpr inline cmplx expic(f32 theta) {
  return cmplx{glm::cos(theta), glm::sin(theta)};
}

template<typename T>
using qua = glm::qua<T>;

using quat = qua<f32>;
using dquat = qua<f64>;

template<typename T>
constexpr qua<T> axisquat(const T& ang, const ::shogle::math::vec<3, T>& axis) {
  return qua<T>{glm::cos(ang * T{0.5}), glm::sin(ang * T{0.5}) * axis};
}

template<typename T>
constexpr qua<T> eulerquat(const ::shogle::math::vec<3, T>& rot) {
  // return
  //   axisquat(rot.x, ::shogle::math::vec<3, T>{T{1}, T{0}, T{0}}) *
  //   axisquat(rot.y, ::shogle::math::vec<3, T>{T{0}, T{1}, T{0}}) *
  //   axisquat(rot.z, ::shogle::math::vec<3, T>{T{0}, T{0}, T{1}});

  const T cp = glm::cos(rot.x * T{0.5});
  const T sp = glm::sin(rot.x * T{0.5});
  const T cy = glm::cos(rot.y * T{0.5});
  const T sy = glm::sin(rot.y * T{0.5});
  const T cr = glm::cos(rot.z * T{0.5});
  const T sr = glm::sin(rot.z * T{0.5});

  qua<T> q;

  q.w = cr * cp * cy + sr * sp * sy;
  q.x = sr * cp * cy - cr * sp * sy;
  q.y = cr * sp * cy + sr * cp * sy;
  q.z = cr * cp * sy - sr * sp * cy;

  return q;
}

template<typename T>
constexpr ::shogle::math::vec<3, T> eulerquat(const qua<T>& q) noexcept {
  ::shogle::math::vec<3, T> euler;

  // Pitch
  const T sinr_cosp = T{2} * (q.w * q.x + q.y * q.z);
  const T cosr_cosp = T{1} - T{2} * (q.x * q.x + q.y * q.y);
  euler.x = std::atan2(sinr_cosp, cosr_cosp);

  // Yaw
  const T sinp = std::sqrt(T{1} + T{2} * (q.w * q.y - q.x * q.z));
  const T cosp = std::sqrt(T{1} - T{2} * (q.w * q.y - q.x * q.z));
  euler.y = T{2} * std::atan2(sinp, cosp) - glm::pi<T>() / T{2};

  // Roll
  const T siny_cosp = T{2} * (q.w * q.z + q.x * q.y);
  const T cosy_cosp = T{1} - T{2} * (q.y * q.y + q.z * q.z);
  euler.z = std::atan2(siny_cosp, cosy_cosp);

  return euler;
}

template<typename T>
mat<4, 4, T> build_trs_matrix(const ::shogle::math::vec<3, T>& pos,
                              const ::shogle::math::vec<3, T>& scale,
                              const ::shogle::math::vec<3, T>& pivot, const qua<T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, pos + pivot);
  m *= glm::mat4_cast(rot);
  m = glm::translate(m, -pivot);
  m = glm::scale(m, scale);
  return m;
}

template<typename T>
mat<4, 4, T> build_trs_matrix(const ::shogle::math::vec<2, T>& pos,
                              const ::shogle::math::vec<2, T>& scale,
                              const ::shogle::math::vec<2, T>& pivot, const qua<T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, ::shogle::math::vec<3, T>{pos.x + pivot.x, pos.y + pivot.y, T{0}});
  m *= glm::mat4_cast(rot);
  m = glm::translate(m, ::shogle::math::vec<3, T>{-pivot.x, -pivot.y, T{0}});
  m = glm::scale(m, ::shogle::math::vec<3, T>{scale.x, scale.y, T{1}});
  return m;
}

template<typename T>
mat<4, 4, T>
build_trs_matrix(const ::shogle::math::vec<3, T>& pos, const ::shogle::math::vec<3, T>& scale,
                 const ::shogle::math::vec<3, T>& pivot, const ::shogle::math::vec<3, T>& offset,
                 const qua<T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, pos + pivot);
  m *= glm::mat4_cast(rot);
  m = glm::translate(m, offset - pivot);
  m = glm::scale(m, scale);
  m = glm::translate(m, -offset);
  return m;
}

template<typename T>
mat<4, 4, T>
build_trs_matrix(const ::shogle::math::vec<2, T>& pos, const ::shogle::math::vec<2, T>& scale,
                 const ::shogle::math::vec<2, T>& pivot, const ::shogle::math::vec<2, T>& offset,
                 const qua<T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, ::shogle::math::vec<3, T>{pos.x + pivot.x, pos.y + pivot.y, T{0}});
  m *= glm::mat4_cast(rot);
  m = glm::translate(m, ::shogle::math::vec<3, T>{offset.x - pivot.x, offset.x - pivot.y, T{0}});
  m = glm::scale(m, ::shogle::math::vec<3, T>{scale.x, scale.y, T{1}});
  m = glm::translate(m, ::shogle::math::vec<3, T>{-offset.x, -offset.y, T{0}});
  return m;
}

} // namespace shogle::math
