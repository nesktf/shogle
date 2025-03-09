#pragma once

#include "../math/matrix.hpp"

#include "../stl/optional.hpp"
#include "../stl/expected.hpp"

#include "../stl/ptr.hpp"

#include <glad/glad.h>

#if SHOGLE_ENABLE_IMGUI
#include <imgui.h>
#include <imgui_impl_opengl3.h> // Should work fine with OGL 4.x
#endif

#if SHOGLE_USE_GLFW
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#if SHOGLE_ENABLE_IMGUI
#include <imgui_impl_glfw.h>
#endif
#endif

#define SHOGLE_DECLARE_RENDER_HANDLE(_name) \
namespace ntf { \
class _name { \
public: \
  constexpr _name() : _handle{r_handle_tombstone} {} \
  constexpr explicit _name(r_handle_value handle) : _handle{handle} {} \
public: \
  constexpr explicit operator r_handle_value() const noexcept { return _handle; } \
  constexpr bool valid() const noexcept { return _handle != r_handle_tombstone; } \
  constexpr explicit operator bool() const noexcept { return valid(); } \
  constexpr bool operator==(const _name& rhs) const noexcept { \
    return _handle == static_cast<r_handle_value>(rhs); \
  } \
private: \
  r_handle_value _handle; \
}; \
} \
namespace std { \
template<> \
struct hash<::ntf::_name> { \
  std::size_t operator()(const ::ntf::_name& h) const noexcept { \
    return hash<::ntf::r_handle_value>{}(static_cast<::ntf::r_handle_value>(h)); \
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
class r_context_view;

enum class r_win_api : uint8 {
  glfw,
  sdl2, // ?
};
class r_window;

using r_handle_value = uint32;
constexpr r_handle_value r_handle_tombstone = std::numeric_limits<r_handle_value>::max();

using r_error = ::ntf::error<void>;

template<typename T>
using r_expected = ::ntf::expected<T, r_error>;

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

enum class r_shader_type : uint8 {
  vertex = 0,
  fragment,
  geometry,
  tesselation_eval,
  tesselation_control,
  compute,
};

enum class r_stages_flag : uint8 {
  none                = 0,
  vertex              = 1 << static_cast<uint8>(r_shader_type::vertex),
  fragment            = 1 << static_cast<uint8>(r_shader_type::fragment),
  geometry            = 1 << static_cast<uint8>(r_shader_type::geometry),
  tesselation_eval    = 1 << static_cast<uint8>(r_shader_type::tesselation_eval),
  tesselation_control = 1 << static_cast<uint8>(r_shader_type::tesselation_control),
  compute             = 1 << static_cast<uint8>(r_shader_type::compute),
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(r_stages_flag);
constexpr r_stages_flag r_required_render_stages = r_stages_flag::vertex | r_stages_flag::fragment;

enum class r_texture_type : uint8 {
  texture1d = 0,
  texture2d,
  texture3d,
  cubemap,
};

enum class r_texture_format : uint8 {
  r8n = 0, r8u,     r8i,
  r16u,    r16i,    r16f,
  r32u,    r32i,    r32f,

  rg8n,    rg8u,    rg8i,
  rg16u,   rg16i,   rg16f,
  rg32u,   rg32i,   rg32f,

  rgb8n,   rgb8u,   rgb8i,
  rgb16u,  rgb16i,  rgb16f,
  rgb32u,  rgb32i,  rgb32f,

  rgba8n,  rgba8u,  rgba8i,
  rgba16u, rgba16i, rgba16f,
  rgba32u, rgba32i, rgba32f,

  srgb8u,  srgba8u,
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
constexpr inline uint32 r_cubemap_layer(r_cubemap_face face) {
  return static_cast<uint32>(face);
}

enum class r_buffer_type : uint8 {
  vertex = 0,
  index,
  texel,
  uniform,
  shader_storage,
};

enum class r_buffer_flag : uint8 {
  none              = 0,
  dynamic_storage   = 1 << 0,
  // read_mappable     = 1 << 1,
  // write_mappable    = 1 << 2,
  // rw_mappable       = (1<<1) | (1<<2),
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(r_buffer_flag);

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

enum class r_test_buffer_format : uint8 {
  depth16u = 0,
  depth24u,
  depth32f,
  depth24u_stencil8u,
  depth32f_stencil8u,
};

enum class r_test_buffer_flag : uint8 {
  none    = 0,
  depth   = 1 << 0,
  stencil = 1 << 1,
  both    = (1<<0) | (1<<1),
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(r_test_buffer_flag);

enum class r_pipeline_test : uint8 {
  none          = 0,
  depth         = 1 << 0,
  stencil       = 1 << 1,
  scissor       = 1 << 2,
  depth_stencil = (1<<0) | (1<<1),
  all           = (1<<0) | (1<<1) | (1<<2),
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(r_pipeline_test);

enum class r_clear_flag : uint8 {
  none          = 0,
  color         = 1 << 0,
  depth         = 1 << 1,
  stencil       = 1 << 2,
  color_depth   = (1<<0) | (1<<1),
  color_stencil = (1<<0) | (1<<2),
  all           = (1<<0) | (1<<1) | (1<<2),
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(r_clear_flag);

struct r_attrib_binding {
  uint32 binding;
  size_t stride;
};

struct r_attrib_descriptor {
  r_attrib_type type;
  uint32 binding;
  uint32 location;
  size_t offset;
};

struct r_buffer_data {
  const void* data;
  size_t size;
  size_t offset;
};

struct r_buffer_descriptor {
  r_buffer_type type;
  r_buffer_flag flags;
  size_t size;

  weak_ref<r_buffer_data> data;
};

struct r_image_data {
  const void* texels;
  r_texture_format format;

  uvec3 extent;
  uvec3 offset;

  uint32 layer;
  uint32 level;
};

struct r_texture_descriptor {
  r_texture_type type;
  r_texture_format format;

  uvec3 extent;
  uint32 layers;
  uint32 levels;

  span_view<r_image_data> images;
  bool gen_mipmaps{false};
  r_texture_sampler sampler;
  r_texture_address addressing;
};

struct r_texture_data {
  span_view<r_image_data> images;
  bool gen_mipmaps{false};
  optional<r_texture_sampler> sampler;
  optional<r_texture_address> addressing;
};

struct r_shader_descriptor {
  r_shader_type type;
  span_view<std::string_view> source;
};

struct r_pipeline_descriptor {
  span_view<r_shader_handle> stages;

  weak_ref<r_attrib_binding> attrib_binding;
  span_view<r_attrib_descriptor> attrib_desc;

  r_primitive primitive;
  r_polygon_mode poly_mode;
  r_front_face front_face;
  r_cull_mode cull_mode;

  r_pipeline_test tests;
  optional<r_compare_op> depth_compare_op;
  optional<r_compare_op> stencil_compare_op;
};

struct r_framebuffer_attachment {
  r_texture_handle handle;
  uint32 layer;
  uint32 level;
};

struct r_framebuffer_descriptor {
  uvec2 extent;
  uvec4 viewport;
  color4 clear_color;
  r_clear_flag clear_flags;

  r_test_buffer_flag test_buffers;
  optional<r_test_buffer_format> test_buffer_format;

  span_view<r_framebuffer_attachment> attachments;
  optional<r_texture_format> color_buffer_format;
};

struct r_draw_opts {
  uint32 count;
  uint32 offset;
  uint32 instances;
  uint32 sort_group;
};

struct r_buffer_binding {
  r_buffer_handle buffer;
  r_buffer_type type;
  optional<uint32> location;
};

struct r_texture_binding {
  r_texture_handle texture;
  uint32 location;
};

struct r_push_constant {
  r_uniform location;
  const void* data;
  r_attrib_type type;
  size_t alignment;
};

struct r_draw_command {
  r_framebuffer_handle target;
  r_pipeline_handle pipeline;
  span_view<r_buffer_binding> buffers;
  span_view<r_texture_binding> textures;
  span_view<r_push_constant> uniforms;
  r_draw_opts draw_opts;
  std::function<void(r_context_view)> on_render;
};

template<typename T>
struct r_attrib_traits {
  static constexpr bool is_attrib = false;
};

template<typename T>
concept is_shader_attribute = r_attrib_traits<T>::is_attrib;

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

SHOGLE_DECLARE_ATTRIB_TRAIT(float32,  r_attrib_type::f32);
SHOGLE_DECLARE_ATTRIB_TRAIT(vec2,     r_attrib_type::vec2);
SHOGLE_DECLARE_ATTRIB_TRAIT(vec3,     r_attrib_type::vec3);
SHOGLE_DECLARE_ATTRIB_TRAIT(vec4,     r_attrib_type::vec4);
SHOGLE_DECLARE_ATTRIB_TRAIT(mat3,     r_attrib_type::mat3);
SHOGLE_DECLARE_ATTRIB_TRAIT(mat4,     r_attrib_type::mat4);

SHOGLE_DECLARE_ATTRIB_TRAIT(float64,  r_attrib_type::f64);
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec2,    r_attrib_type::dvec2);
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec3,    r_attrib_type::dvec3);
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec4,    r_attrib_type::dvec4);
SHOGLE_DECLARE_ATTRIB_TRAIT(dmat3,    r_attrib_type::dmat3);
SHOGLE_DECLARE_ATTRIB_TRAIT(dmat4,    r_attrib_type::dmat4);

SHOGLE_DECLARE_ATTRIB_TRAIT(int32,    r_attrib_type::i32);
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec2,    r_attrib_type::ivec2);
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec3,    r_attrib_type::ivec3);
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec4,    r_attrib_type::ivec4);

template<is_shader_attribute T>
constexpr r_push_constant r_fmt_pconst(r_uniform location, const T& data) {
  return r_push_constant{
    .location = location,
    .data = std::addressof(data),
    .type = r_attrib_traits<T>::tag,
    .alignment = alignof(T),
  };
}

} // namespace ntf

#undef SHOGLE_DECLARE_ATTRIB_TRAIT
#undef SHOGLE_DECLARE_RENDER_HANDLE
