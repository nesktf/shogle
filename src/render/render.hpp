#pragma once

#include "../math/matrix.hpp"

#ifdef SHOGLE_ENABLE_IMGUI
#include <imgui.h>
#endif

#define SHOGLE_DECLARE_RENDER_HANDLE(_name) \
namespace ntf { \
class _name { \
public: \
  constexpr _name() : _handle{r_handle_tombstone} {} \
  constexpr explicit _name(r_handle_value handle) : _handle{handle} {} \
public: \
  constexpr r_handle_value value() const { return _handle; } \
  constexpr operator r_handle_value() const { return value(); } \
  constexpr bool valid() const { return _handle == r_handle_tombstone; } \
  constexpr explicit operator bool() const { return valid(); } \
private: \
  r_handle_value _handle; \
}; \
} \
namespace std { \
template<> \
struct hash<::ntf::_name> { \
  std::size_t operator()(const ::ntf::_name& h) const noexcept { \
    return hash<::ntf::r_handle_value>{}(h.value()); \
  } \
}; \
}

#define SHOGLE_DECLARE_ATTRIB_TRAIT(_type, _tag) \
template<> \
struct r_attrib_traits<_type> { \
  static constexpr auto tag = _tag; \
  static constexpr size_t size = r_attrib_type_size(tag); \
  static constexpr uint32 dim = r_attrib_type_dim(tag); \
  static constexpr bool is_attrib = true; \
}

namespace ntf {

using color3 = vec3;
using color4 = vec4;

enum class r_api : uint8 {
  software,
  opengl,
  vulkan,
};
class r_context;

enum class r_win_api : uint8 {
  glfw,
  sdl2, // ?
};
class r_window;

using r_handle_value = uint32;
constexpr r_handle_value r_handle_tombstone = std::numeric_limits<r_handle_value>::max();

} // namespace ntf

SHOGLE_DECLARE_RENDER_HANDLE(r_buffer_handle);
SHOGLE_DECLARE_RENDER_HANDLE(r_texture_handle);
SHOGLE_DECLARE_RENDER_HANDLE(r_shader_handle);
SHOGLE_DECLARE_RENDER_HANDLE(r_pipeline_handle);
SHOGLE_DECLARE_RENDER_HANDLE(r_framebuffer_handle);
SHOGLE_DECLARE_RENDER_HANDLE(r_uniform);

namespace ntf {

enum class r_attrib_type : uint32 {
  f32, vec2,  vec3,  vec4,  mat3,  mat4,
  f64, dvec2, dvec3, dvec4, dmat3, dmat4,
  i32, ivec2, ivec3, ivec4,
};

template<typename T>
struct r_attrib_traits {
  static constexpr bool is_attrib = false;
};

constexpr inline size_t r_attrib_type_size(r_attrib_type type) {
  constexpr size_t int_sz = sizeof(int32);
  constexpr size_t float_sz = sizeof(float);
  constexpr size_t double_sz = sizeof(double);

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
  };

  return 0;
};

constexpr inline uint32 r_attrib_type_dim(r_attrib_type type) {
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
  };

  return 0;
}

SHOGLE_DECLARE_ATTRIB_TRAIT(float32, r_attrib_type::f32);
SHOGLE_DECLARE_ATTRIB_TRAIT(vec2, r_attrib_type::vec2);
SHOGLE_DECLARE_ATTRIB_TRAIT(vec3, r_attrib_type::vec3);
SHOGLE_DECLARE_ATTRIB_TRAIT(vec4, r_attrib_type::vec4);
SHOGLE_DECLARE_ATTRIB_TRAIT(mat3, r_attrib_type::mat3);
SHOGLE_DECLARE_ATTRIB_TRAIT(mat4, r_attrib_type::mat4);

SHOGLE_DECLARE_ATTRIB_TRAIT(float64, r_attrib_type::f64);
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec2, r_attrib_type::dvec2);
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec3, r_attrib_type::dvec3);
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec4, r_attrib_type::dvec4);
SHOGLE_DECLARE_ATTRIB_TRAIT(dmat3, r_attrib_type::dmat3);
SHOGLE_DECLARE_ATTRIB_TRAIT(dmat4, r_attrib_type::dmat4);

SHOGLE_DECLARE_ATTRIB_TRAIT(int32, r_attrib_type::i32);
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec2, r_attrib_type::ivec2);
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec3, r_attrib_type::ivec3);
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec4, r_attrib_type::ivec4);

enum class r_shader_type : uint8 {
  vertex = 0,
  fragment,
  geometry,
  tesselation_eval,
  tesselation_control,
  compute,
};

enum class r_shader_usage_flag : uint8 {
  none                = 0,
  vertex              = 1 << static_cast<uint8>(r_shader_type::vertex),
  fragment            = 1 << static_cast<uint8>(r_shader_type::fragment),
  geometry            = 1 << static_cast<uint8>(r_shader_type::geometry),
  tesselation_eval    = 1 << static_cast<uint8>(r_shader_type::tesselation_eval),
  tesselation_control = 1 << static_cast<uint8>(r_shader_type::tesselation_control),
  compute             = 1 << static_cast<uint8>(r_shader_type::compute),
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(r_shader_usage_flag)

struct r_shader_descriptor {
  std::string_view  source;
  r_shader_type     type;
};

enum class r_texture_type : uint8 {
  texture1d = 0,
  texture1d_array,
  texture2d,
  texture2d_array,
  texture3d,
  cubemap,
};
constexpr uint8 r_texture_type_count = 6;

enum class r_texture_format : uint8 {
  mono = 0,
  rgb,
  rgba,
};

enum class r_texture_sampler : uint8 {
  nearest = 0,
  linear,
};

enum class r_texture_address : uint8 {
  repeat = 0,
  repeat_mirrored,
  clamp_edge,
  clamp_edge_mirrored,
  clamp_border,
};

enum class r_cubemap_face : uint8 {
  positive_x = 0,
  negative_x,
  positive_y,
  negative_y,
  positive_z,
  negative_z,
};
constexpr uint8 r_cubemap_face_count = 6;

struct r_texture_descriptor {
  void const* const*  texels;
  uint32              count;
  uint32              mipmaps;
  uvec3               extent;
  r_texture_type      type;
  r_texture_format    format;
  r_texture_sampler   sampler;
  r_texture_address   addressing;
};

enum class r_buffer_type : uint8 {
  vertex = 0,
  index,
  texel,
  uniform,
  shader_storage,
};
constexpr uint8 r_buffer_type_count = 5;

struct r_buffer_descriptor {
  const void*   data;
  size_t        size;
  r_buffer_type type;
};

enum class r_primitive : uint8 {
  triangles = 0,
  triangle_strip,
  triangle_fan,
  lines,
  line_strip,
  points,
};

enum class r_polygon_mode : uint8 {
  fill = 0,
  point,
  line,
};

enum class r_compare_op : uint8 {
  never = 0,
  always,
  less,
  greater,
  equal,
  lequal,
  gequal,
  nequal,
};

enum class r_front_face : uint8 {
  clockwise = 0,
  counter_clockwise,
};

enum class r_cull_mode : uint8 {
  front = 0,
  back,
  front_back,
};

struct r_attrib_descriptor {
  uint32        binding;
  uint32        location;
  size_t        offset;
  r_attrib_type type;
};

struct r_pipeline_descriptor {
  const r_shader_handle*      stages;
  uint32                      stage_count;

  const r_attrib_descriptor*  attribs;
  uint32                      attrib_count;
  size_t                      stride;

  r_primitive                 primitive;
  r_polygon_mode              poly_mode;
  bool                        depth_test;
  r_compare_op                depth_compare_op;
  // bool                        depth_write;
  r_front_face                front_face;
  r_cull_mode                 cull_mode;
};

struct r_framebuffer_descriptor {
  uvec2 size;
  const r_texture_handle* attachments;
  uint32 attachment_count;
};

enum class r_clear_flag : uint8 {
  none    = 0,
  color   = 1 << 0,
  depth   = 1 << 1,
  stencil = 1 << 2,
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(r_clear_flag);

struct r_draw_opts {
  uint32 draw_count;
  uint32 draw_offset;
  uint32 instance_count;
};

} // namespace ntf

#undef SHOGLE_DECLARE_ATTRIB_TRAIT
#undef SHOGLE_DECLARE_RENDER_HANDLE
