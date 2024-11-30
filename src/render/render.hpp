#pragma once

#include "../math/alg.hpp"

#include <map>

namespace ntf {

enum class shader_data_type {
  none = 0,
  f32, vec2,  vec3,  vec4,  mat3,  mat4,
  f64, dvec2, dvec3, dvec4, dmat3, dmat4,
  i32, ivec2, ivec3, ivec4,
};

static inline std::size_t shader_data_size(shader_data_type type) {
  constexpr std::size_t int_sz = sizeof(int);
  constexpr std::size_t float_sz = sizeof(float);
  constexpr std::size_t double_sz = sizeof(double);

  switch (type) {
    case shader_data_type::i32:   return int_sz;
    case shader_data_type::ivec2: return 2*int_sz;
    case shader_data_type::ivec3: return 3*int_sz;
    case shader_data_type::ivec4: return 4*int_sz;

    case shader_data_type::f32:   return float_sz;
    case shader_data_type::vec2:  return 2*float_sz;
    case shader_data_type::vec3:  return 3*float_sz;
    case shader_data_type::vec4:  return 4*float_sz;
    case shader_data_type::mat3:  return 9*float_sz;
    case shader_data_type::mat4:  return 16*float_sz;

    case shader_data_type::f64:   return double_sz;
    case shader_data_type::dvec2: return 2*double_sz;
    case shader_data_type::dvec3: return 3*double_sz;
    case shader_data_type::dvec4: return 4*double_sz;
    case shader_data_type::dmat3: return 9*double_sz;
    case shader_data_type::dmat4: return 16*double_sz;

    case shader_data_type::none: break;
  };

  NTF_ASSERT(false, "Invalid shader data type");
  return 0;
};

static inline uint shader_data_count(shader_data_type type) {
  switch (type) {
    case shader_data_type::i32:   [[fallthrough]];
    case shader_data_type::f32:   [[fallthrough]];
    case shader_data_type::f64:   return 1;

    case shader_data_type::vec2:  [[fallthrough]];
    case shader_data_type::ivec2: [[fallthrough]];
    case shader_data_type::dvec2: return 2;

    case shader_data_type::vec3:  [[fallthrough]];
    case shader_data_type::ivec3: [[fallthrough]];
    case shader_data_type::dvec3: return 3;

    case shader_data_type::vec4:  [[fallthrough]];
    case shader_data_type::ivec4: [[fallthrough]];
    case shader_data_type::dvec4: return 4;

    case shader_data_type::mat3:  [[fallthrough]];
    case shader_data_type::dmat3: return 9;

    case shader_data_type::mat4:  [[fallthrough]];
    case shader_data_type::dmat4: return 16;

    case shader_data_type::none:  break;
  };

  NTF_ASSERT(false, "Invalid shader data type");
  return 0;
}

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
  none = 0,
  vertex,
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

template<typename F>
concept framebuffer_func = std::invocable<F>;

} // namespace ntf
