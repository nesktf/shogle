#pragma once

#include "./math.hpp"
#include <ntfstl/concepts.hpp>

namespace ntf {

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

using color3 = vec3;
using color4 = vec4;

using extent1d = uint32;
using extent2d = uvec2;
using extent3d = uvec3;

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

} // namespace ntf
