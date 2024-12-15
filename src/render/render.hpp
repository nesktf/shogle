#pragma once

#include "../math/alg.hpp"

#include <map>
#include <vector>

namespace ntf {

enum class w_backend : std::uint8_t {
  none = 0,
  glfw,
  sdl2, // ?
};

enum class r_backend : std::uint8_t {
  none = 0,
  software,
  opengl,
  vulkan,
};

struct r_extent_2D {
  std::uint32_t width{0};
  std::uint32_t height{0};
};

struct r_extent_3D {
  std::uint32_t width{0};
  std::uint32_t height{0};
  std::uint32_t depth{0};
};

enum class r_attrib_type : std::uint32_t {
  none = 0,
  f32, vec2,  vec3,  vec4,  mat3,  mat4,
  f64, dvec2, dvec3, dvec4, dmat3, dmat4,
  i32, ivec2, ivec3, ivec4,
};

constexpr inline std::size_t r_attrib_size(r_attrib_type type) {
  constexpr std::size_t int_sz = sizeof(int32_t);
  constexpr std::size_t float_sz = sizeof(float);
  constexpr std::size_t double_sz = sizeof(double);

  switch (type) {
    case r_attrib_type::i32:   return int_sz;
    case r_attrib_type::ivec2: return 2*int_sz;
    case r_attrib_type::ivec3: return 3*int_sz;
    case r_attrib_type::ivec4: return 4*int_sz;

    case r_attrib_type::f32:   return float_sz;
    case r_attrib_type::vec2:  return 2*float_sz;
    case r_attrib_type::vec3:  return 3*float_sz;
    case r_attrib_type::vec4:  return 4*float_sz;
    case r_attrib_type::mat3:  return 9*float_sz;
    case r_attrib_type::mat4:  return 16*float_sz;

    case r_attrib_type::f64:   return double_sz;
    case r_attrib_type::dvec2: return 2*double_sz;
    case r_attrib_type::dvec3: return 3*double_sz;
    case r_attrib_type::dvec4: return 4*double_sz;
    case r_attrib_type::dmat3: return 9*double_sz;
    case r_attrib_type::dmat4: return 16*double_sz;

    case r_attrib_type::none:  return 0;
  };

  return 0;
};

constexpr inline std::uint32_t r_attrib_dim(r_attrib_type type) {
  switch (type) {
    case r_attrib_type::i32:   [[fallthrough]];
    case r_attrib_type::f32:   [[fallthrough]];
    case r_attrib_type::f64:   return 1;

    case r_attrib_type::vec2:  [[fallthrough]];
    case r_attrib_type::ivec2: [[fallthrough]];
    case r_attrib_type::dvec2: return 2;

    case r_attrib_type::vec3:  [[fallthrough]];
    case r_attrib_type::ivec3: [[fallthrough]];
    case r_attrib_type::dvec3: return 3;

    case r_attrib_type::vec4:  [[fallthrough]];
    case r_attrib_type::ivec4: [[fallthrough]];
    case r_attrib_type::dvec4: return 4;

    case r_attrib_type::mat3:  [[fallthrough]];
    case r_attrib_type::dmat3: return 9;

    case r_attrib_type::mat4:  [[fallthrough]];
    case r_attrib_type::dmat4: return 16;

    case r_attrib_type::none:  return 0;
  };

  return 0;
}

struct r_attrib_info {
  std::uint32_t   binding{0};
  std::uint32_t   location{0};
  std::size_t     offset{0};
  r_attrib_type   type{r_attrib_type::none};
};

enum class r_shader_type : std::uint8_t {
  none        = 0,
  vertex      = 1 << 0,
  fragment    = 1 << 1,
  geometry    = 1 << 2,
  tesselation = 1 << 3,
  compute     = 1 << 4,
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(r_shader_type)

struct r_shader_info {
  std::string_view  source{};
  r_shader_type     type{r_shader_type::none};
};

template<typename RenderContext>
class r_shader;

enum class r_texture_type : std::uint8_t {
  none = 0,
  texture1d,
  texture2d,
  texture3d,
  cubemap,
};

enum class r_texture_format : std::uint8_t {
  none = 0,
  mono,
  rgb,
  rgba,
};

enum class r_texture_sampler : std::uint8_t {
  none = 0,
  nearest,
  linear,
};

enum class r_texture_address : std::uint8_t {
  none = 0,
  repeat,
  repeat_mirrored,
  clamp_edge,
  clamp_edge_mirrored,
  clamp_border,
};

struct r_texture_info {
  const uint8_t**   texels{nullptr};
  std::uint32_t     count{0};
  std::uint32_t     mipmap_level{0};
  r_extent_3D       extent{};
  r_texture_type    type{r_texture_type::none};
  r_texture_format  format{r_texture_format::none};
  r_texture_sampler sampler{r_texture_sampler::none};
  r_texture_address addressing{r_texture_address::none};
};

template<typename RenderContext>
class r_texture;

enum class r_buffer_type : std::uint8_t {
  none = 0,
  vertex,
  index,
  uniform,
};

struct r_buffer_info {
  const void*   data{nullptr};
  std::size_t   size{0};
  r_buffer_type type{r_buffer_type::none};
};

template<typename RenderContext>
class r_buffer;

enum class r_primitive : std::uint8_t {
  none = 0,
  triangles,
  triangle_strip,
  triangle_fan,
  lines,
  line_strip,
  points,
};

enum class r_polygon_mode : std::uint8_t {
  none = 0,
  point,
  line,
  fill,
};

template<typename RenderContext>
struct r_pipeline_info {
  const r_shader<RenderContext>*  stages;
  const r_attrib_info*            attribs{nullptr};
  std::uint32_t                   attrib_count{0};
  r_primitive                     primitive{r_primitive::none};
  r_polygon_mode                  poly_mode{r_polygon_mode::none};
};

template<typename RenderContext>
class r_pipeline;


template<typename RenderContext>
struct r_mesh_info {

};

template<typename RenderContext>
class r_mesh;


enum class r_clear : std::uint8_t {
  none    = 0,
  color   = 1 << 0,
  depth   = 1 << 1,
  stencil = 1 << 2,
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(r_clear)




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
