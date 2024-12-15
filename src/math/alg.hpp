#pragma once

#include "../core.hpp"
#include "../stl/common.hpp"

#ifdef SHOGLE_USE_GLM
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#endif

#include <complex>
#include <cmath>

// #define PI M_PIf
// #define I cmplx{0.0f, 1.0f}

namespace ntf {

#ifdef SHOGLE_USE_GLM
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using mat3 = glm::mat3;
using ivec2 = glm::ivec2;
using ivec3 = glm::ivec3;
using quat = glm::quat;
#endif

using cmplx = std::complex<float>;

using color = vec4;
using color4 = vec4;
using color3 = vec3;

template<typename T>
concept vertex_type = (ntf::same_as_any<T, vec2, vec3, vec4>);

inline vec3 carcoord(vec3 v) { return {v.z, v.x, v.y}; }

inline vec3 glcoord(vec3 v) { return {v.y, v.z, v.x}; }

inline vec3 carcoord(float rho, float theta, float phi) {
  return rho*vec3{glm::sin(theta)*glm::cos(phi), glm::sin(theta)*glm::sin(phi), glm::cos(theta)};
}

inline vec3 glcoord(float rho, float theta, float phi) {
  return glcoord(carcoord(rho, theta, phi));
}

inline quat axisquat(float ang, vec3 axis) {
  return quat{glm::cos(ang*0.5f), glm::sin(ang*0.5f)*axis};
}

inline quat eulerquat(vec3 rot) {
  return
    axisquat(rot.x, vec3{1.0f, 0.0f, 0.0f}) *
    axisquat(rot.y, vec3{0.0f, 1.0f, 0.0f}) *
    axisquat(rot.z, vec3{0.0f, 0.0f, 1.0f});
};

inline vec2 expiv(float theta) {
  return vec2{glm::cos(theta), glm::sin(theta)};
}

inline cmplx expic(float theta) {
  return cmplx{glm::cos(theta), glm::sin(theta)};
}

inline float rad(float deg) {
  return glm::radians(deg);
}

inline float deg(float rad) {
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
inline bool equal(T f1, T f2) { 
  return (std::fabs(f1 - f2) <= std::numeric_limits<T>::epsilon() * std::fmax(std::fabs(f1), std::fabs(f2)));
}

template<typename T>
T epserr(const T& a, const T& b) {
  return std::abs((b-a)/b);
}

constexpr float norm2(cmplx z) {
  return z.real()*z.real() + z.imag()*z.imag();
}

constexpr float norm2(vec2 v) {
  return v.x*v.x + v.y*v.y;
}

constexpr float norm(cmplx z) {
  return glm::sqrt(norm2(z));
}

constexpr float norm(vec2 v) {
  return glm::sqrt(norm2(v));
}

constexpr cmplx normalize(cmplx z) {
  return z/glm::sqrt(norm2(z));
}

constexpr vec2 normalize(vec2 v) {
  return v/glm::sqrt(norm2(v));
}

constexpr cmplx conv(const vec2& v) {
  return cmplx{v.x, v.y};
}

constexpr vec2 conv(const cmplx& z) {
  return vec2{z.real(), z.imag()};
}

} // namespace ntf
