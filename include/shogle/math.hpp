#pragma once

#include <shogle/core.hpp>

#include <ntfstl/concepts.hpp>
#include <ntfstl/types.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <complex>
#include <cmath>

namespace ntf {

constexpr inline f32 rad(f32 deg) {
  return glm::radians(deg);
}

constexpr inline f32 deg(f32 rad) {
  return glm::degrees(rad);
}

template<typename TL, typename TR>
constexpr auto periodic_add(TL a, TR b, decltype(a+b) min, decltype(a+b) max) {
  auto res = a + b;
  auto range = max-min;
  while (res >= max) res -= range;
  while (res <  min) res += range;
  return res;
}

template<typename T>
constexpr bool equal(T f1, T f2) { 
  return (std::fabs(f1 - f2) <= std::numeric_limits<T>::epsilon() * std::fmax(std::fabs(f1),
                                                                              std::fabs(f2)));
}

template<typename T>
constexpr T epserr(const T& a, const T& b) {
  return std::abs((b-a)/b);
}


template<uint32 N, typename T>
using vec = glm::vec<N, T>;

using vec2 = vec<2, f32>;
using vec3 = vec<3, f32>;
using vec4 = vec<4, f32>;

using dvec2 = vec<2, float64>;
using dvec3 = vec<3, float64>;
using dvec4 = vec<4, float64>;

using ivec2 = vec<2, int32>;
using ivec3 = vec<3, int32>;
using ivec4 = vec<4, int32>;

using uvec2 = vec<2, uint32>;
using uvec3 = vec<3, uint32>;
using uvec4 = vec<4, uint32>;

template<typename T>
concept vertex_type = (ntf::meta::same_as_any<T, vec2, vec3, vec4>);

constexpr inline vec3 carcoord(vec3 v) { return {v.z, v.x, v.y}; }

constexpr inline vec3 glcoord(vec3 v) { return {v.y, v.z, v.x}; }

constexpr inline vec3 carcoord(float rho, float theta, float phi) {
  return rho*vec3{glm::sin(theta)*glm::cos(phi), glm::sin(theta)*glm::sin(phi), glm::cos(theta)};
}

constexpr inline vec3 glcoord(float rho, float theta, float phi) {
  return glcoord(carcoord(rho, theta, phi));
}

constexpr inline vec2 expiv(float theta) {
  return vec2{glm::cos(theta), glm::sin(theta)};
}

constexpr inline f32 norm2(vec2 v) {
  return v.x*v.x + v.y*v.y;
}

constexpr inline f32 norm(vec2 v) {
  return glm::sqrt(norm2(v));
}

constexpr inline vec2 normalize(vec2 v) {
  return v/glm::sqrt(norm2(v));
}

template<uint32 N, uint32 M, typename T>
using mat = glm::mat<N, M, T>;

using mat3 = mat<3, 3, f32>;
using mat4 = mat<4, 4, f32>;

using dmat3 = mat<3, 3, f64>;
using dmat4 = mat<4, 4, f64>;

template<typename T>
mat<4, 4, T> build_trs_matrix(const vec<2, T>& pos,
                              const vec<2, T>& scale,
                              const vec<3, T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, vec<3, T>{pos, T{0}});
  m = glm::rotate(m, rot.z, vec<3, T>{T{0}, T{0}, T{1}});
  m = glm::rotate(m, rot.y, vec<3, T>{T{0}, T{1}, T{0}});
  m = glm::rotate(m, rot.x, vec<3, T>{T{1}, T{0}, T{0}});
  m = glm::scale(m, vec<3, T>{scale, T{1}});
  return m;
}

template<typename T>
mat<4, 4, T> build_trs_matrix(const vec<3, T>& pos,
                              const vec<3, T>& scale,
                              const vec<3, T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, pos);
  m = glm::rotate(m, rot.z, vec<3, T>{T{0}, T{0}, T{1}});
  m = glm::rotate(m, rot.y, vec<3, T>{T{0}, T{1}, T{0}});
  m = glm::rotate(m, rot.x, vec<3, T>{T{1}, T{0}, T{0}});
  m = glm::scale(m, scale);
  return m;
}

template<typename T>
mat<4, 4, T> build_trs_matrix(const vec<2, T>& pos,
                              const vec<2, T>& scale,
                              const vec<2, T>& pivot,
                              const vec<3, T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, vec<3, T>{pos.x+pivot.x, pos.y+pivot.y, T{0}});
  m = glm::rotate(m, rot.z, vec<3, T>{T{0}, T{0}, T{1}});
  m = glm::rotate(m, rot.y, vec<3, T>{T{0}, T{1}, T{0}});
  m = glm::rotate(m, rot.x, vec<3, T>{T{1}, T{0}, T{0}});
  m = glm::translate(m, vec<3, T>{-pivot.x, -pivot.y, T{0}});
  m = glm::scale(m, vec<3, T>{scale.x, scale.y, T{1}});
  return m;
}

template<typename T>
mat<4, 4, T> build_trs_matrix(const vec<3, T>& pos,
                              const vec<3, T>& scale,
                              const vec<3, T>& pivot,
                              const vec<3, T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, pos+pivot);
  m = glm::rotate(m, rot.z, vec<3, T>{T{0}, T{0}, T{1}});
  m = glm::rotate(m, rot.y, vec<3, T>{T{0}, T{1}, T{0}});
  m = glm::rotate(m, rot.x, vec<3, T>{T{1}, T{0}, T{0}});
  m = glm::translate(m, -pivot);
  m = glm::scale(m, scale);
  return m;
}

template<typename T>
mat<4, 4, T> build_trs_matrix(const vec<2, T>& pos,
                              const vec<2, T>& scale,
                              const vec<2, T>& pivot,
                              const vec<2, T>& offset,
                              const vec<3, T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, vec<3, T>{pos.x+pivot.x, pos.y+pivot.y, T{0}});
  m = glm::rotate(m, rot.z, vec<3, T>{T{0}, T{0}, T{1}});
  m = glm::rotate(m, rot.y, vec<3, T>{T{0}, T{1}, T{0}});
  m = glm::rotate(m, rot.x, vec<3, T>{T{1}, T{0}, T{0}});
  m = glm::translate(m, vec<3, T>{offset.x-pivot.x, offset.x-pivot.y, T{0}});
  m = glm::scale(m, vec<3, T>{scale.x, scale.y, T{1}});
  m = glm::translate(m, vec<3, T>{-offset.x, -offset.y, T{0}});
  return m;
}

template<typename T>
mat<4, 4, T> build_trs_matrix(const vec<3, T>& pos,
                              const vec<3, T>& scale,
                              const vec<3, T>& pivot,
                              const vec<3, T>& offset,
                              const vec<3, T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, pos+pivot);
  m = glm::rotate(m, rot.z, vec<3, T>{T{0}, T{0}, T{1}});
  m = glm::rotate(m, rot.y, vec<3, T>{T{0}, T{1}, T{0}});
  m = glm::rotate(m, rot.x, vec<3, T>{T{1}, T{0}, T{0}});
  m = glm::translate(m, offset-pivot);
  m = glm::scale(m, scale);
  return m;
}


template<typename T>
mat<4, 4, T> build_view_matrix(const vec<2, T>& pos,
                               const vec<2, T>& origin,
                               const vec<2, T>& zoom,
                               const vec<3, T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, vec<3, T>{origin.x, origin.y, T{0}});
  m = glm::rotate(m, rot.z, vec<3, T>{T{0}, T{0}, T{1}});
  m = glm::rotate(m, rot.y, vec<3, T>{T{0}, T{1}, T{0}});
  m = glm::rotate(m, rot.x, vec<3, T>{T{1}, T{0}, T{0}});
  m = glm::scale(m, vec<3, T>{zoom.x, zoom.y, T{1}});
  m = glm::translate(m, vec<3, T>{-pos.x, -pos.y, T{0}});
  return m;
}

template<typename T>
mat<4, 4, T> build_view_matrix(const vec<2, T>& pos,
                               const vec<2, T>& origin,
                               const vec<2, T>& zoom,
                               const T& rot,
                               const vec<3, T>& axis) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, vec<3, T>{origin.x, origin.y, T{0}});
  m = glm::rotate(m, rot, axis);
  m = glm::scale(m, vec<3, T>{zoom.x, zoom.y, T{1}});
  m = glm::translate(m, vec<3, T>{-pos.x, -pos.y, T{0}});
  return m;
}

template<typename T>
mat<4, 4, T> build_view_matrix(const vec<3, T>& pos,
                               const vec<3, T>& dir,
                               const vec<3, T>& up) noexcept {
  return glm::lookAt(pos, pos+dir, up);
}

template<typename T>
mat<4, 4, T> build_proj_matrix(const vec<2, T>& viewport,
                               const T& znear,
                               const T& zfar) noexcept {
  return glm::ortho(T{0}, viewport.x,
                    viewport.y, T{0},
                    znear, zfar);
}

template<typename T>
mat<4, 4, T> build_proj_matrix(const vec<2, T>& viewport,
                               const T& znear,
                               const T& zfar,
                               const T& fov) noexcept {
  return glm::perspective(fov, viewport.x/viewport.y, znear, zfar);
}

template<typename T>
using complex = std::complex<T>;

using cmplx = complex<f32>;
using dcmplx = complex<f64>;
using icmplx = complex<int32>;
using ucmplx = complex<uint32>;

constexpr inline f32 norm2(cmplx z) {
  return z.real()*z.real() + z.imag()*z.imag();
}

constexpr inline cmplx conv(const vec2& v) {
  return cmplx{v.x, v.y};
}

constexpr inline vec2 conv(const cmplx& z) {
  return vec2{z.real(), z.imag()};
}

constexpr inline cmplx normalize(cmplx z) {
  return z/glm::sqrt(norm2(z));
}

constexpr inline float norm(cmplx z) {
  return glm::sqrt(norm2(z));
}

constexpr inline cmplx expic(float theta) {
  return cmplx{glm::cos(theta), glm::sin(theta)};
}

template<typename T>
using qua = glm::qua<T>;

using quat = qua<f32>;
using dquat = qua<f64>;

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

  const T cp = glm::cos(rot.x*T{0.5});
  const T sp = glm::sin(rot.x*T{0.5});
  const T cy = glm::cos(rot.y*T{0.5});
  const T sy = glm::sin(rot.y*T{0.5});
  const T cr = glm::cos(rot.z*T{0.5});
  const T sr = glm::sin(rot.z*T{0.5});

  qua<T> q;

  q.w = cr*cp*cy + sr*sp*sy;
  q.x = sr*cp*cy - cr*sp*sy;
  q.y = cr*sp*cy + sr*cp*sy;
  q.z = cr*cp*sy - sr*sp*cy;

  return q;
}

template<typename T>
constexpr vec<3, T> eulerquat(const qua<T>& q) noexcept {
  vec<3, T> euler;

  // Pitch
  const T sinr_cosp = T{2}*(q.w*q.x + q.y*q.z);
  const T cosr_cosp = T{1} - T{2}*(q.x*q.x + q.y*q.y);
  euler.x = std::atan2(sinr_cosp, cosr_cosp);

  // Yaw
  const T sinp = std::sqrt(T{1} + T{2}*(q.w*q.y - q.x*q.z));
  const T cosp = std::sqrt(T{1} - T{2}*(q.w*q.y - q.x*q.z));
  euler.y = T{2}*std::atan2(sinp, cosp) - glm::pi<T>()/T{2};

  // Roll
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
  m = glm::translate(m, pos+pivot);
  m*= glm::mat4_cast(rot);
  m = glm::translate(m, -pivot);
  m = glm::scale(m, scale);
  return m;
}

template<typename T>
mat<4, 4, T> build_trs_matrix(const vec<2, T>& pos,
                              const vec<2, T>& scale,
                              const vec<2, T>& pivot,
                              const qua<T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, vec<3, T>{pos.x+pivot.x, pos.y+pivot.y, T{0}});
  m*= glm::mat4_cast(rot);
  m = glm::translate(m, vec<3, T>{-pivot.x, -pivot.y, T{0}});
  m = glm::scale(m, vec<3, T>{scale.x, scale.y, T{1}});
  return m;
}

template<typename T>
mat<4, 4, T> build_trs_matrix(const vec<3, T>& pos,
                              const vec<3, T>& scale,
                              const vec<3, T>& pivot,
                              const vec<3, T>& offset,
                              const qua<T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, pos+pivot);
  m*= glm::mat4_cast(rot);
  m = glm::translate(m, offset-pivot);
  m = glm::scale(m, scale);
  m = glm::translate(m, -offset);
  return m;
}

template<typename T>
mat<4, 4, T> build_trs_matrix(const vec<2, T>& pos,
                              const vec<2, T>& scale,
                              const vec<2, T>& pivot,
                              const vec<2, T>& offset,
                              const qua<T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, vec<3, T>{pos.x+pivot.x, pos.y+pivot.y, T{0}});
  m*= glm::mat4_cast(rot);
  m = glm::translate(m, vec<3, T>{offset.x-pivot.x, offset.x-pivot.y, T{0}});
  m = glm::scale(m, vec<3, T>{scale.x, scale.y, T{1}});
  m = glm::translate(m, vec<3, T>{-offset.x, -offset.y, T{0}});
  return m;
}

constexpr inline bool collision2d(vec2 pos1, vec2 size1, vec2 pos2, vec2 size2) {
  // AABB assumes pos is the lower left corner
  // Normalize scale, since obj.pos it's the quad's center
  vec2 normsize1 = size1 / 2.0f;
  vec2 normsize2 = size2 / 2.0f;

  bool col_x = 
    pos1.x + normsize1.x >= (pos2.x - normsize2.x) &&
    pos2.x + normsize2.x >= (pos1.x - normsize1.x);
  bool col_y = 
    pos1.y + normsize1.y >= (pos2.y - normsize2.y) &&
    pos2.y + normsize2.y >= (pos1.y - normsize1.y);

  return col_x && col_y;
}

constexpr inline bool collision2d(vec2 pos1, vec2 size1, vec2 pos2, float rad2) {
  // No need to normalize pos1 to be the center
  float sq_rad = rad2*rad2; // mult is cheaper than sqrt?
  vec2 half_rect = size1 / 2.0f;

  vec2 diff = pos2 - pos1;
  vec2 diff_clamp = glm::clamp(diff, -half_rect, half_rect);
  vec2 closest_p = pos1 + diff_clamp;

  diff = closest_p - pos2;
  float sq_len = (diff.x*diff.x) + (diff.y*diff.y);

  return sq_len < sq_rad;
}

constexpr inline bool collision2d(vec2 pos1, float rad1, vec2 pos2, vec2 size2) {
  return collision2d(pos2, size2, pos1, rad1);
}

constexpr inline bool collision2d(vec2 pos1, float rad1, vec2 pos2, float rad2) {
  vec2 diff = pos2 - pos1;

  float sq_sum = (rad1+rad2)*(rad1+rad2); // maybe it is
  float sq_len = (diff.x*diff.x) + (diff.y*diff.y);

  return sq_len < sq_sum;
}


// TODO: integrator traits?
template<typename Fun, typename T>
concept integr_fun = std::is_invocable_r_v<T, Fun, T>;
// f(T x) -> T

template<typename T>
struct integr_trap {
  template<integr_fun<T> Fun>
  constexpr T operator()(T a, T b, Fun&& f) {
    const T size = b-a;
    return (size/2)*(f(a)+f(b));
  }

  template<integr_fun<T> Fun>
  constexpr T operator()(T a, T b, uint n, Fun&& f) {
    const T size = b-a;
    const T h = size/static_cast<T>(n);

    T out = f(a);
    for (uint i = 1; i < n; ++i) {
      out += 2*f(a+(i*h));
    }
    out += f(b);

    return (size*out)/(2*n);
  }
};

template<typename T>
struct integr_simp13 {
  template<integr_fun<T> Fun>
  constexpr T operator()(T a, T b, Fun&& f) {
    const T size = b-a;
    const T x_1 = a+((b-a)/3);

    return (size/6)*(f(a)+4*f(x_1)+f(b));
  }

  template<integr_fun<T> Fun>
  constexpr T operator()(T a, T b, uint n, Fun&& f) {
    const T size = b-a;
    const T h = size/static_cast<T>(n);

    T out = f(a);
    for (uint i = 1; i < n; i+=2) {
      out += 4*f(a+(i*h)); // Odd
    }
    for (uint i = 2; i < n; i+=2) {
      out += 2*f(a+(i*h)); // Even
    }
    out += f(b);

    return (size*out)/(3*n);
  }
};

template<typename T>
struct integr_simp38 {
  template<integr_fun<T> Fun>
  constexpr T operator()(T a, T b, Fun&& f) {
    const T size = b-a;
    const T h = size/4;
    const T x_1 = a+h;
    const T x_2 = a+(h*2);

    return (size/8)*(f(a)+3*f(x_1)+3*f(x_2)+f(b));
  }
};


// TODO: ODE system solvers
template<typename Fun, typename T>
concept ode_fun = std::is_invocable_r_v<T, Fun, T, T>;
// f(T x, T y) -> T
// \phi(x, y, h) = f(x, y)

template<typename T>
struct ode_rk4 {
  template<ode_fun<T> Fun>
  constexpr T operator()(T x, T y, T h, Fun&& f) {
    const auto k1 = h*f(x, y);
    const auto k2 = h*f(x + h*T{0.5}, y + k1*T{0.5});
    const auto k3 = h*f(x + h*T{0.5}, y + k2*T{0.5});
    const auto k4 = h*f(x + h, y + k3);

    return y + T{1/6.0}*(k1 + 2*k2 + 2*k3 + k4);
  }
};

template<typename T>
struct ode_euler {
  template<ode_fun<T> Fun>
  constexpr T operator()(T x, T y, T h, Fun&& f) {
    return y + h*f(x, y);
  }
};

} // namespace ntf
