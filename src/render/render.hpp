#pragma once

#include "../math/alg.hpp"

#include <map>

namespace ntf {

enum class renderer_backend {
  software = 0,
  opengl,
  vulkan,
};

enum class clear : uint8_t {
  none = 0,
  depth = 1 << 0,
  stencil = 1 << 1,
};

constexpr clear operator|(clear a, clear b) {
  return static_cast<clear>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

constexpr clear operator&(clear a, clear b) {
  return static_cast<clear>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

constexpr clear& operator|=(clear& a, clear b) {
  return a = static_cast<clear>(a | b);
}

enum class depth_fun {
  less = 0,
  lequal,
};

enum class tex_format {
  mono = 0,
  rgb,
  rgba,
};

enum class tex_filter {
  nearest = 0,
  linear,
  nearest_mp_nearest,
  nearest_mp_linear,
  linear_mp_linear,
  linear_mp_nearest,
};

enum class tex_wrap {
  repeat = 0,
  mirrored_repeat,
  clamp_edge,
  clamp_border,
};

enum class shader_category {
  vertex = 0,
  fragment,
  geometry
};

enum class uniform_category {
  scalar = 0, // float
  iscalar, // int
  vec2,
  vec3, 
  vec4,
  mat3,
  mat4,
};

enum class mesh_buffer {
  static_draw = 0,
  dynamic_draw,
  stream_draw,
};

enum class mesh_primitive {
  triangles = 0,
  triangle_strip,
  triangle_fan,
  lines,
  line_strip,
  line_loop,
  points,
};

enum class material_category {
  diffuse = 0,
  specular
};

struct font_glyph {
  ivec2 size;
  ivec2 bearing;
  unsigned long advance;
};

using glyph_map = std::map<uint8_t, std::pair<uint8_t*, font_glyph>>;

template<unsigned int Index, vertex_type T>
struct shader_attribute {
  static constexpr unsigned int index = Index;
  static constexpr std::size_t stride = sizeof(T);
};

template<class T>
concept is_shader_attribute = requires(T x) { 
  { shader_attribute{x} } -> std::same_as<T>;
};

template<typename T>
struct uniform_traits {
  static constexpr bool is_uniform = false;
};

template<>
struct uniform_traits<float> {
  static constexpr bool is_uniform = true;
  static constexpr uniform_category enum_value = uniform_category::scalar;
  static constexpr std::size_t size = sizeof(float);
};

template<>
struct uniform_traits<int> {
  static constexpr bool is_uniform = true;
  static constexpr uniform_category enum_value = uniform_category::iscalar;
  static constexpr std::size_t size = sizeof(int);
};

template<>
struct uniform_traits<vec2> {
  static constexpr bool is_uniform = true;
  static constexpr uniform_category enum_value = uniform_category::vec2;
  static constexpr std::size_t size = sizeof(vec2);
};

template<>
struct uniform_traits<vec3> {
  static constexpr bool is_uniform = true;
  static constexpr uniform_category enum_value = uniform_category::vec3;
  static constexpr std::size_t size = sizeof(vec3);
};

template<>
struct uniform_traits<vec4> {
  static constexpr bool is_uniform = true;
  static constexpr uniform_category enum_value = uniform_category::vec4;
  static constexpr std::size_t size = sizeof(vec4);
};

template<>
struct uniform_traits<mat3> {
  static constexpr bool is_uniform = true;
  static constexpr uniform_category enum_value = uniform_category::mat3;
  static constexpr std::size_t size = sizeof(mat3);
};

template<>
struct uniform_traits<mat4> {
  static constexpr bool is_uniform = true;
  static constexpr uniform_category enum_value = uniform_category::mat4;
  static constexpr std::size_t size = sizeof(mat4);
};

} // namespace ntf
