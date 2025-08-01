#pragma once

#if defined(SHOGLE_EXPOSE_GLFW) && SHOGLE_EXPOSE_GLFW
#include <glad/glad.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

#include <shogle/math.hpp>

// #include "../../src/render/window.hpp"
//
// #if defined(SHOGLE_ENABLE_IMGUI) && SHOGLE_ENABLE_IMGUI
// #if defined(SHOGLE_EXPOSE_GLFW) && SHOGLE_EXPOSE_GLFW
// #include <imgui_impl_opengl3.h>
// #include <imgui_impl_glfw.h>
// #endif
// #include "../../src/render/imgui.hpp"
// #endif


namespace ntf::render {

using color3 = vec3;
using color4 = vec4;

using extent1d = uint32;
using extent2d = uvec2;
using extent3d = uvec3;

NTF_DECLARE_OPAQUE_HANDLE(context_t);
NTF_DECLARE_OPAQUE_HANDLE(window_t);
NTF_DECLARE_OPAQUE_HANDLE(texture_t);
NTF_DECLARE_OPAQUE_HANDLE(buffer_t);
NTF_DECLARE_OPAQUE_HANDLE(shader_t);
NTF_DECLARE_OPAQUE_HANDLE(pipeline_t);
NTF_DECLARE_OPAQUE_HANDLE(framebuffer_t);
NTF_DECLARE_OPAQUE_HANDLE(uniform_t);

enum class context_api {
  none = 0,
  opengl,
  vulkan,
  software,
};

using ctx_handle = uint64;
constexpr ctx_handle CTX_HANDLE_TOMB = std::numeric_limits<ctx_handle>::max();

template<typename T>
using cstring_view = std::basic_string_view<T, std::char_traits<T>>;

class render_error : public std::exception {
public:
  render_error(cstring_view<char> str) noexcept :
    _str{str} {}

public:
  const char* what() const noexcept override { return _str.data(); }
  cstring_view<char> msg() const { return _str; }

private:
  cstring_view<char> _str;
};

template<typename T>
using expect = ::ntf::expected<T, render_error>;

enum class image_format : uint8 {
  r8nu=0,  r8n,     r8u,     r8i,
  r16u,    r16i,    r16f,
  r32u,    r32i,    r32f,

  rg8nu,   rg8n,    rg8u,    rg8i,
  rg16u,   rg16i,   rg16f,
  rg32u,   rg32i,   rg32f,

  rgb8nu,  rgb8n,   rgb8u,   rgb8i,
  rgb16u,  rgb16i,  rgb16f,
  rgb32u,  rgb32i,  rgb32f,

  rgba8nu, rgba8n,  rgba8u,  rgba8i,
  rgba16u, rgba16i, rgba16f,
  rgba32u, rgba32i, rgba32f,

  srgb8u,  srgba8u,
};

using image_alignment = size_t; // usually 1, 2, 4, or 8

struct image_data {
  const void* bitmap;
  image_format format;
  image_alignment alignment;
  extent3d extent;
  extent3d offset;
  uint32 layer;
  uint32 level;
};

constexpr uint8 IMAGE_DEPTH_CHANNELS_MASK = 0b00000111;
constexpr uint8 IMAGE_DEPTH_NORMALIZE_BIT = 0b10000000;
constexpr uint8 IMAGE_DEPTH_NONLINEAR_BIT = 0b01000000;

enum class primitive_mode : uint8 {
  triangles = 0,
  triangle_strip,
  triangle_fan,
  lines,
  line_strip,
  points,
};

enum class polygon_mode : uint8 {
  fill = 0,
  point,
  line,
};

enum class test_func : uint8 {
  never = 0,
  always,
  less,
  greater,
  equal,
  lequal,
  gequal,
  nequal,
};

enum class stencil_op : uint8 {
  keep = 0,
  set_zero,
  replace,
  incr,
  incr_wrap,
  decr,
  decr_wrap,
  invert,
};

enum class cull_face : uint8 {
  clockwise = 0,
  counter_clockwise,
};

enum class cull_mode : uint8 {
  front = 0,
  back,
  front_back,
};

enum class blend_mode : uint8 {
  min = 0,
  max,
  add,
  subs,
  rev_subs,
};

enum class blend_factor : uint8 {
  zero = 0,
  one,

  src_color,
  inv_src_color,
  dst_color,
  inv_dst_color,

  src1_color,
  inv_src1_color,

  src_alpha,
  inv_src_alpha,
  dst_alpha,
  inv_dst_alpha,
  src_alpha_sat,

  src1_alpha,
  inv_src1_alpha,

  const_color,
  inv_const_color,
  const_alpha,
  inv_const_alpha,
};

struct blend_opts {
  blend_mode mode;
  blend_factor src_factor;
  blend_factor dst_factor;
  color4 color;
};

struct stencil_rule {
  stencil_op on_stencil_fail;
  stencil_op on_depth_fail;
  stencil_op on_pass;
};

struct stencil_func {
  test_func func;
  int32 ref;
  uint32 mask;
};

struct stencil_test_opts {
  stencil_func func;
  stencil_rule rule;
  uint32 mask;
};

struct depth_test_opts {
  test_func func;
  f64 near_bound;
  f64 far_bound;
};

struct scissor_test_opts {
  extent2d pos;
  extent2d size;
};

struct face_cull_opts {
  cull_mode mode;
  cull_face front_face;
};

struct render_tests {
  weak_cptr<stencil_test_opts> stencil_test;
  weak_cptr<depth_test_opts> depth_test;
  weak_cptr<scissor_test_opts> scissor_test;
  weak_cptr<face_cull_opts> face_culling;
  weak_cptr<blend_opts> blending;
};

enum class clear_flag : uint8 {
  none          = 0,
  color         = 1 << 0,
  depth         = 1 << 1,
  stencil       = 1 << 2,
  color_depth   = (1<<0) | (1<<1),
  color_stencil = (1<<0) | (1<<2),
  all           = (1<<0) | (1<<1) | (1<<2),
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(clear_flag);
} // namespace ntf::render


namespace ntf::meta {

template<typename T>
struct image_depth_traits {
  static constexpr bool is_specialized = false;
};

NTF_DECLARE_TAG_TYPE(image_depth_u8);
template<>
struct image_depth_traits<uint8> {
  static constexpr bool is_specialized = true;

  using tag_type = image_depth_u8_t;
  static constexpr tag_type tag = image_depth_u8;

  static constexpr bool is_signed = false;
  static constexpr bool is_floating = false;
  static constexpr std::string_view name = "8 bit unsigned";

  static constexpr optional<ntf::render::image_format> parse_channels(uint8 flags) noexcept {
    const uint8 channels = flags & ntf::render::IMAGE_DEPTH_CHANNELS_MASK;
    if (flags & ntf::render::IMAGE_DEPTH_NORMALIZE_BIT) {
      switch (channels) {
        case 1u: return ntf::render::image_format::r8nu;
        case 2u: return ntf::render::image_format::rg8nu;
        case 3u: return ntf::render::image_format::rgb8nu;
        case 4u: return ntf::render::image_format::rgba8nu;
      }
    } else if (flags & ntf::render::IMAGE_DEPTH_NONLINEAR_BIT) {
      switch (channels) {
        case 3u: return ntf::render::image_format::srgb8u;
        case 4u: return ntf::render::image_format::srgba8u;
      }
    } else {
      switch (channels) {
        case 1u: return ntf::render::image_format::r8u;
        case 2u: return ntf::render::image_format::rg8u;
        case 3u: return ntf::render::image_format::rgb8u;
        case 4u: return ntf::render::image_format::rgba8u;
      }
    }
    return nullopt;
  }
};

NTF_DECLARE_TAG_TYPE(image_depth_s8);
template<>
struct image_depth_traits<int8> {
  static constexpr bool is_specialized = true;

  using tag_type = image_depth_s8_t;
  static constexpr tag_type tag = image_depth_s8;

  static constexpr bool is_signed = true;
  static constexpr bool is_floating = false;
  static constexpr std::string_view name = "8 bit signed";

  static constexpr optional<ntf::render::image_format> parse_channels(uint8 flags) noexcept {
    const uint8 channels = flags & ntf::render::IMAGE_DEPTH_CHANNELS_MASK;
    if (flags & ntf::render::IMAGE_DEPTH_NORMALIZE_BIT) {
      switch (channels) {
        case 1u: return ntf::render::image_format::r8n;
        case 2u: return ntf::render::image_format::rg8n;
        case 3u: return ntf::render::image_format::rgb8n;
        case 4u: return ntf::render::image_format::rgba8n;
      }
    } else {
      switch (channels) {
        case 1u: return ntf::render::image_format::r8i;
        case 2u: return ntf::render::image_format::rg8i;
        case 3u: return ntf::render::image_format::rgb8i;
        case 4u: return ntf::render::image_format::rgba8i;
      }
    }
    return nullopt;
  }
};

NTF_DECLARE_TAG_TYPE(image_depth_u16);
template<>
struct image_depth_traits<uint16> {
  static constexpr bool is_specialized = true;

  using tag_type = image_depth_u16_t;
  static constexpr tag_type tag = image_depth_u16;

  static constexpr bool is_signed = false;
  static constexpr bool is_floating = false;
  static constexpr std::string_view name = "16 bit unsigned";

  static constexpr optional<ntf::render::image_format> parse_channels(uint8 flags) noexcept {
    const uint8 channels = flags & ntf::render::IMAGE_DEPTH_CHANNELS_MASK;
    switch (channels) {
      case 1u: return ntf::render::image_format::r16u;
      case 2u: return ntf::render::image_format::rg16u;
      case 3u: return ntf::render::image_format::rgb16u;
      case 4u: return ntf::render::image_format::rgba16u;
    }
    return nullopt;
  }
};

NTF_DECLARE_TAG_TYPE(image_depth_s16);
template<>
struct image_depth_traits<int16> {
  static constexpr bool is_specialized = true;

  using tag_type = image_depth_s16_t;
  static constexpr tag_type tag = image_depth_s16;

  static constexpr bool is_signed = true;
  static constexpr bool is_floating = false;
  static constexpr std::string_view name = "16 bit signed";

  static constexpr optional<ntf::render::image_format> parse_channels(uint8 flags) noexcept {
    const uint8 channels = flags & ntf::render::IMAGE_DEPTH_CHANNELS_MASK;
    switch (channels) {
      case 1u: return ntf::render::image_format::r16i;
      case 2u: return ntf::render::image_format::rg16i;
      case 3u: return ntf::render::image_format::rgb16i;
      case 4u: return ntf::render::image_format::rgba16i;
    }
    return nullopt;
  }
};

NTF_DECLARE_TAG_TYPE(image_depth_f32);
template<>
struct image_depth_traits<float32> {
  static constexpr bool is_specialized = true;

  using tag_type = image_depth_f32_t;
  static constexpr tag_type tag = image_depth_f32;

  static constexpr bool is_signed = true;
  static constexpr bool is_floating = true;
  static constexpr std::string_view name = "32 bit float";

  static constexpr optional<ntf::render::image_format> parse_channels(uint8 flags) noexcept {
    const uint8 channels = flags & ntf::render::IMAGE_DEPTH_CHANNELS_MASK;
    switch (channels) {
      case 1u: return ntf::render::image_format::r32f;
      case 2u: return ntf::render::image_format::rg32f;
      case 3u: return ntf::render::image_format::rgb32f;
      case 4u: return ntf::render::image_format::rgba32f;
    }
    return nullopt;
  }
};

template<typename T>
concept image_depth_type = image_depth_traits<T>::is_specialized;

template<typename T>
struct image_dim_traits {
  static constexpr bool is_specialized = false;
};

template<>
struct image_dim_traits<ntf::render::extent1d> {
  static constexpr bool is_specialized = true;
  static constexpr bool allows_arrays = true;

  template<image_depth_type T>
  constexpr static size_t image_stride(ntf::render::extent1d extent) noexcept {
    return static_cast<size_t>(extent)*sizeof(T);
  }
  constexpr static ntf::render::extent3d extent_cast(ntf::render::extent1d extent) noexcept {
    return {static_cast<uint32>(extent), 1u, 1u};
  }
  constexpr static ntf::render::extent3d offset_cast(ntf::render::extent1d offset, uint32 layer = 0u) noexcept {
    return {static_cast<uint32>(offset), layer, 0u};
  }
};

template<>
struct image_dim_traits<ntf::render::extent2d> {
  static constexpr bool is_specialized = true;
  static constexpr bool allows_arrays = true;

  template<image_depth_type T>
  constexpr static size_t image_stride(ntf::render::extent2d extent) noexcept {
    return static_cast<size_t>(extent.x*extent.y)*sizeof(T);
  }
  constexpr static ntf::render::extent3d extent_cast(ntf::render::extent2d extent) noexcept {
    return {static_cast<uint32>(extent.x), static_cast<uint32>(extent.y), 1u};
  }
  constexpr static ntf::render::extent3d offset_cast(ntf::render::extent2d offset, uint32 layer = 0u) noexcept {
    return {static_cast<uint32>(offset.x), static_cast<uint32>(offset.y), layer};
  }
};

template<>
struct image_dim_traits<ntf::render::extent3d> {
  static constexpr bool is_specialized = true;
  static constexpr bool allows_arrays = false;

  template<image_depth_type T>
  constexpr static size_t image_stride(ntf::render::extent3d extent) noexcept {
    return static_cast<size_t>(extent.x*extent.y*extent.y)*sizeof(T);
  }
  constexpr static ntf::render::extent3d extent_cast(ntf::render::extent3d extent) noexcept {
    return extent;
  }
  constexpr static ntf::render::extent3d offset_cast(ntf::render::extent3d offset) noexcept {
    return offset;
  }
};

template<typename T>
concept image_dim_type = image_dim_traits<T>::is_specialized;

template<typename T>
concept image_array_dim_type = image_dim_type<T> && image_dim_traits<T>::allows_arrays;

} // namespace ntf::meta

#define SHOGLE_DECLARE_ATTRIB_TRAIT(_type, _tag, _underlying, _get_ptr) \
static_assert(std::is_trivial_v<_type>, NTF_STRINGIFY(_type) " is not trivial!!!"); \
template<> \
struct attribute_traits<_type> { \
  static constexpr bool is_specialized = true; \
  using underlying_type = _underlying; \
  static constexpr ntf::render::attribute_type tag = _tag; \
  static constexpr size_t size = attribute_size(tag); \
  static constexpr uint32 dim = attribute_dim(tag); \
  static const _underlying* value_ptr(const _type& obj) noexcept { \
    return _get_ptr; \
  } \
}

namespace ntf::render {

enum class attribute_type : uint32 {
  f32, vec2,  vec3,  vec4,  mat3,  mat4,
  f64, dvec2, dvec3, dvec4, dmat3, dmat4,
  i32, ivec2, ivec3, ivec4,
};

struct attribute_binding {
  attribute_type type;
  uint32 location;
  size_t offset;
  size_t stride;
};

// For uniforms (who cares about double precision matrices??? Use an uniform buffer!!!!!)
using attribute_data = ntf::inplace_trivial<sizeof(mat4), alignof(mat4)>;

} // namespace ntf::render

namespace ntf::meta {

template<typename T>
struct attribute_traits {
  static constexpr bool is_specialized = false;
};

constexpr inline size_t attribute_size(ntf::render::attribute_type type) noexcept {
  constexpr size_t int_sz = sizeof(int32);
  constexpr size_t f32_sz = sizeof(f32);
  constexpr size_t f64_sz = sizeof(f64);

  switch (type) {
    case ntf::render::attribute_type::i32:   return int_sz;
    case ntf::render::attribute_type::ivec2: return 2*int_sz;
    case ntf::render::attribute_type::ivec3: return 3*int_sz;
    case ntf::render::attribute_type::ivec4: return 4*int_sz;

    case ntf::render::attribute_type::f32:   return f32_sz;
    case ntf::render::attribute_type::vec2:  return 2*f32_sz;
    case ntf::render::attribute_type::vec3:  return 3*f32_sz;
    case ntf::render::attribute_type::vec4:  return 4*f32_sz;
    case ntf::render::attribute_type::mat3:  return 9*f32_sz;
    case ntf::render::attribute_type::mat4:  return 16*f32_sz;

    case ntf::render::attribute_type::f64:   return f64_sz;
    case ntf::render::attribute_type::dvec2: return 2*f64_sz;
    case ntf::render::attribute_type::dvec3: return 3*f64_sz;
    case ntf::render::attribute_type::dvec4: return 4*f64_sz;
    case ntf::render::attribute_type::dmat3: return 9*f64_sz;
    case ntf::render::attribute_type::dmat4: return 16*f64_sz;
  };

  return 0;
};

constexpr inline uint32 attribute_dim(ntf::render::attribute_type type) noexcept {
  switch (type) {
    case ntf::render::attribute_type::i32:   [[fallthrough]];
    case ntf::render::attribute_type::f32:   [[fallthrough]];
    case ntf::render::attribute_type::f64:   return 1;

    case ntf::render::attribute_type::vec2:  [[fallthrough]];
    case ntf::render::attribute_type::ivec2: [[fallthrough]];
    case ntf::render::attribute_type::dvec2: return 2;

    case ntf::render::attribute_type::vec3:  [[fallthrough]];
    case ntf::render::attribute_type::ivec3: [[fallthrough]];
    case ntf::render::attribute_type::dvec3: return 3;

    case ntf::render::attribute_type::vec4:  [[fallthrough]];
    case ntf::render::attribute_type::ivec4: [[fallthrough]];
    case ntf::render::attribute_type::dvec4: return 4;

    case ntf::render::attribute_type::mat3:  [[fallthrough]];
    case ntf::render::attribute_type::dmat3: return 9;

    case ntf::render::attribute_type::mat4:  [[fallthrough]];
    case ntf::render::attribute_type::dmat4: return 16;
  };

  return 0;
}

SHOGLE_DECLARE_ATTRIB_TRAIT(f32, ntf::render::attribute_type::f32, f32, &obj);
SHOGLE_DECLARE_ATTRIB_TRAIT(vec2, ntf::render::attribute_type::vec2, f32, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(vec3, ntf::render::attribute_type::vec3, f32, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(vec4, ntf::render::attribute_type::vec4, f32, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(mat3, ntf::render::attribute_type::mat3, f32, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(mat4, ntf::render::attribute_type::mat4, f32, glm::value_ptr(obj));

SHOGLE_DECLARE_ATTRIB_TRAIT(f64, ntf::render::attribute_type::f64, f64, &obj);
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec2, ntf::render::attribute_type::dvec2, f64, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec3, ntf::render::attribute_type::dvec3, f64, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec4, ntf::render::attribute_type::dvec4, f64, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(dmat3, ntf::render::attribute_type::dmat3, f64, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(dmat4, ntf::render::attribute_type::dmat4, f64, glm::value_ptr(obj));

SHOGLE_DECLARE_ATTRIB_TRAIT(int32, ntf::render::attribute_type::i32, int32, &obj);
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec2, ntf::render::attribute_type::ivec2, int32, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec3, ntf::render::attribute_type::ivec3, int32, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec4, ntf::render::attribute_type::ivec4, int32, glm::value_ptr(obj));

template<typename T>
concept attribute_type = attribute_traits<T>::is_specialized;

} // namespace ntf::meta

#undef SHOGLE_DECLARE_ATTRIB_TRAIT

namespace ntf::render {

enum class buffer_type : uint8 {
  vertex = 0,
  index,
  texel,
  uniform,
  shader_storage,
};

enum class buffer_flag : uint8 {
  none              = 0,
  dynamic_storage   = 1 << 0,
  read_mappable     = 1 << 1,
  write_mappable    = 1 << 2,
  rw_mappable       = (1<<1) | (1<<2),
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(buffer_flag);

struct buffer_data {
  const void* data;
  size_t size;
  size_t offset;
};

template<typename T>
requires(std::is_trivially_copyable_v<T>)
buffer_data format_buffer_data(const T& data, size_t offset = 0u) {
  return {.data = std::addressof(data), .size = sizeof(T), .offset = offset};
}

struct typed_buffer_desc {
  buffer_flag flags;
  size_t size;
  weak_cptr<buffer_data> data;
};

struct buffer_desc {
  buffer_type type;
  buffer_flag flags;
  size_t size;
  weak_cptr<buffer_data> data;
};

expect<buffer_t> create_buffer(context_t ctx, const buffer_desc& desc);
void destroy_buffer(buffer_t buffer) noexcept;

expect<void> buffer_upload(buffer_t buffer, const buffer_data& data);
expect<void> buffer_upload(buffer_t buffer, size_t size, size_t offset, const void* data);
expect<void*> buffer_map(buffer_t buffer, size_t size, size_t offset);
void buffer_unmap(buffer_t buffer, void* mapped);

buffer_type buffer_get_type(buffer_t buffer);
size_t buffer_get_size(buffer_t buffer);
context_t buffer_get_ctx(buffer_t buffer);
ctx_handle buffer_get_id(buffer_t buffer);

template<meta::image_dim_type DimT>
extent3d image_extent_cast(const DimT& extent) noexcept {
  return meta::image_dim_traits<DimT>::extent_cast(extent);
}

template<meta::image_depth_type T, meta::image_dim_type DimT>
size_t image_stride(const DimT& extent) noexcept {
  return meta::image_dim_traits<DimT>::template image_stride<T>(extent);
}

template<meta::image_dim_type DimT>
extent3d image_offset_cast(const DimT& offset) noexcept {
  return meta::image_dim_traits<DimT>::offset_cast(offset);
}

template<meta::image_array_dim_type DimT>
extent3d image_offset_cast(const DimT& offset, uint32 layer) noexcept {
  return meta::image_dim_traits<DimT>::offset_cast(offset, layer);
}

enum class texture_type : uint8 {
  texture1d = 0,
  texture2d,
  texture3d,
  cubemap,
};

enum class texture_sampler : uint8 {
  nearest = 0,
  linear,
};

enum class texture_addressing : uint8 {
  repeat = 0,
  repeat_mirrored,
  clamp_edge,
  clamp_edge_mirrored,
  clamp_border,
};

enum class cubemap_face : uint8 {
  positive_x = 0,
  negative_x,
  positive_y,
  negative_y,
  positive_z,
  negative_z,
};

struct texture_data {
  cspan<image_data> images;
  bool generate_mipmaps;
};

struct typed_texture_desc {
  image_format format;
  texture_sampler sampler;
  texture_addressing addressing;
  extent3d extent;
  uint32 layers;
  uint32 levels;
  weak_cptr<texture_data> data;
};

struct texture_desc {
  texture_type type;
  image_format format;
  texture_sampler sampler;
  texture_addressing addressing;
  extent3d extent;
  uint32 layers;
  uint32 levels;
  weak_cptr<texture_data> data;
};

expect<texture_t> create_texture(context_t ctx, const texture_desc& desc);
void destroy_texture(texture_t tex) noexcept;

expect<void> texture_upload(texture_t tex, const texture_data& data);
expect<void> texture_set_sampler(texture_t tex, texture_sampler sampler);
expect<void> texture_set_addressing(texture_t tex, texture_addressing adressing);

texture_type texture_get_type(texture_t tex);
image_format texture_get_format(texture_t tex);
texture_sampler texture_get_sampler(texture_t tex);
texture_addressing texture_get_addressing(texture_t tex);
extent3d texture_get_extent(texture_t tex);
uint32 texture_get_layers(texture_t tex);
uint32 texture_get_levels(texture_t tex);
context_t texture_get_ctx(texture_t tex);
ctx_handle texture_get_id(texture_t tex);

constexpr inline uint32 to_cubemap_layer(cubemap_face face) {
  return static_cast<uint32>(face);
}

enum class fbo_buffer : uint8 {
  none = 0,
  depth16u,
  depth24u,
  depth32f,
  depth24u_stencil8u,
  depth32f_stencil8u,
};

struct fbo_image {
  texture_t texture;
  uint32 layer;
  uint32 level;
};

struct fbo_image_desc {
  extent2d extent;
  uvec4 viewport;
  color4 clear_color;
  clear_flag clear_flags;
  fbo_buffer test_buffer;
  cspan<fbo_image> images;
};

// struct fbo_color_desc {
//   extent2d extent;
//   uvec4 viewport;
//   color4 clear_color;
//   clear_flag clear_flags;
//   fbo_buffer test_buffer;
//   image_format color_buffer;
// };

expect<framebuffer_t> create_framebuffer(context_t ctx, const fbo_image_desc& desc);
// expect<framebuffer_t> create_framebuffer(context_t ctx, const fbo_color_desc& desc);
void destroy_framebuffer(framebuffer_t fb) noexcept;

void framebuffer_set_clear_flags(framebuffer_t fb, clear_flag flags);
void framebuffer_set_viewport(framebuffer_t fb, const uvec4& vp);
void framebuffer_set_clear_color(framebuffer_t fb, const color4& color);

clear_flag framebuffer_get_clear_flags(framebuffer_t fb);
uvec4 framebuffer_get_viewport(framebuffer_t fb);
color4 framebuffer_get_clear_color(framebuffer_t fb);

framebuffer_t get_default_framebuffer(context_t ctx);
context_t framebuffer_get_ctx(framebuffer_t fb);
ctx_handle framebuffer_get_id(framebuffer_t fb);

enum class shader_type : uint8 {
  vertex = 0,
  fragment,
  geometry,
  tesselation_eval,
  tesselation_control,
  compute,
};

struct shader_desc {
  shader_type type;
  cspan<cstring_view<char>> source;
};

expect<shader_t> create_shader(context_t ctx, const shader_desc& desc);
void destroy_shader(shader_t shader) noexcept;

shader_type shader_get_type(shader_t shader);
context_t shader_get_ctx(shader_t shader);
ctx_handle shader_get_id(shader_t shader);


enum class stages_flag : uint8 {
  none                = 0,
  vertex              = 1 << static_cast<uint8>(shader_type::vertex),
  fragment            = 1 << static_cast<uint8>(shader_type::fragment),
  geometry            = 1 << static_cast<uint8>(shader_type::geometry),
  tesselation_eval    = 1 << static_cast<uint8>(shader_type::tesselation_eval),
  tesselation_control = 1 << static_cast<uint8>(shader_type::tesselation_control),
  compute             = 1 << static_cast<uint8>(shader_type::compute),
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(stages_flag);

constexpr stages_flag MIN_RENDER_STAGES = stages_flag::vertex | stages_flag::fragment;
constexpr stages_flag MIN_COMPUTE_STAGES = stages_flag::compute;

struct pipeline_desc {
  cspan<attribute_binding> attributes;
  cspan<shader_t> stages;
  primitive_mode primitive;
  polygon_mode poly_mode;
  f32 poly_width;
  render_tests tests;
};

expect<pipeline_t> create_pipeline(context_t ctx, const pipeline_desc& desc);
void destroy_pipeline(pipeline_t pipeline) noexcept;

span<uniform_t> pipeline_get_uniforms(pipeline_t pipeline);
uniform_t pipeline_get_uniform(pipeline_t pip, cstring_view<char> name);
size_t pipeline_get_uniform_count(pipeline_t pip);
attribute_type uniform_get_type(uniform_t uniform);
cstring_view<char> uniform_get_name(uniform_t uniform);
u32 uniform_get_location(uniform_t uniform);

stages_flag pipeline_get_stages(pipeline_t pipeline);
context_t pipeline_get_ctx(pipeline_t pipeline);
ctx_handle pipeline_get_id(pipeline_t pipeline);

struct shader_binding {
  buffer_t buffer;
  uint32 binding;
  size_t size;
  size_t offset;
};

struct vertex_binding {
  buffer_t buffer;
  uint32 layout;
};

struct buffer_binding {
  cspan<vertex_binding> vertex;
  buffer_t index;
  cspan<shader_binding> shader;
};

struct texture_binding {
  texture_t texture;
  u32 sampler;
};

struct uniform_const {
  attribute_data data;
  attribute_type type;
  u32 location;
};

struct render_opts {
  uint32 vertex_count;
  uint32 vertex_offset;
  uint32 index_offset;
  uint32 instances;
};

struct render_cmd {
  framebuffer_t target;
  pipeline_t pipeline;
  buffer_binding buffers;
  cspan<texture_binding> textures;
  cspan<uniform_const> consts;
  render_opts opts;
  uint32 sort_group;
  function_view<void(context_t)> render_callback;
};

struct external_state {
  primitive_mode primitive;
  polygon_mode poly_mode;
  f32 poly_width;
  render_tests test;
};

struct external_cmd {
  framebuffer_t target;
  weak_cptr<external_state> state;
  uint32 sort_group;
  function_view<void(context_t, ctx_handle)> render_callback;
};

struct context_gl_params {
  void* gl_ctx;
  void* (*get_proc_address)(void*, const char*);
  void  (*swap_buffers)(void*);
  void  (*make_current)(void*);
  void  (*get_fb_size)(void*, u32*, u32*);
};

// Placeholders
struct context_vk_params {};
struct context_sw_params {};
struct context_no_params {};

struct context_params {
  const void* ctx_params;
  context_api ctx_api;
  uvec4 fb_viewport;
  clear_flag fb_clear_flags;
  color4 fb_clear_color;
  weak_cptr<malloc_funcs> alloc;
};

expect<context_t> create_context(const context_params& params);
void destroy_context(context_t ctx) noexcept;

void start_frame(context_t ctx);
void end_frame(context_t ctx);
void device_wait(context_t ctx);
void submit_render_command(context_t ctx, const render_cmd& cmd);
void submit_external_command(context_t ctx, const external_cmd& cmd);
context_api get_api(context_t ctx);
cstring_view<char> get_name(context_t ctx);


} // namespace ntf::render

namespace ntf::render {

using win_error = render_error;

template<typename T>
using win_expect = ::ntf::expected<T, win_error>;

enum class win_attrib {
  none = 0,
  decorate   = 1 << 1,
  resizable  = 1 << 2,
  floating   = 1 << 3,
  show_focus = 1 << 4,
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(win_attrib);

struct win_x11_params {
  const char* class_name;
  const char* instance_name;
};

struct win_gl_params {
  uint32 ver_major;
  uint32 ver_minor;
  u32 swap_interval;
  uint32 fb_msaa_level;
  ntf::render::fbo_buffer fb_buffer;
  bool fb_use_alpha;
};

struct win_params {
  uint32 width;
  uint32 height;
  const char* title;
  win_attrib attrib;
  context_api renderer_api;
  const void* platform_params;
  const void* renderer_params;
};

enum class win_key : int16 { // Follows GLFW key values
  unknown = -1,
  space = 32, apostrophe = 39,

  comma = 44, minus, period, slash,
  k0, k1, k2, k3, k4, k5, k6, k7, k8, k9,

  semicolon = 59, equal = 61,

  a = 65, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,
  lbracket, backslash, rbracket,

  grave = 96,

  world1 = 161, world2, // ?

  escape = 256, enter, tab, backspace, insert, del, right, left, down, up,
  pgup, pgdown, home, end,

  capslock = 280, scrolllock, numlock, printscr, pause,

  f1 = 290, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,
  f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25,

  kp0 = 320, kp1, kp2, kp3, kp4, kp5, kp6, kp7, kp8, kp9,
  kpdec, kpdiv, kpmul, kpsub, kpadd, kpenter, kpequal,

  lshift = 340, lctrl, lalt, lsuper, rshift, rctrl, ralt, rsuper, menu,
};

using win_keyscancode = int32;

enum class win_button : uint8 { // Follows GLFW button values
  m1 = 0, m2, m3, m4, m5, m6, m7, m8,
};

enum class win_action : uint8 { // Follows GLFW action values
  release = 0,
  press,
  repeat,
};

enum class win_keymod : uint8 { // Follows GLFW mod values
  none     = 0x00,
  shift    = 0x01,
  ctrl     = 0x02,
  alt      = 0x04,
  super    = 0x08,
  capslock = 0x10,
  numlock  = 0x20,
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(win_keymod);

enum class win_mouse_state : int8 {
  normal = 0,
  hidden,
  disabled,
};

struct win_key_data {
  win_key key;
  win_keyscancode scancode;
  win_action action;
  win_keymod mod;
};

struct win_button_data {
  win_button button;
  win_action action;
  win_keymod mod;
};

class window {
private:
  struct callback_handler_t;
  friend callback_handler_t;
  
  template<typename Signature>
  using fun_t = std::function<Signature>;

public:
  using viewport_fun = fun_t<void(window&, extent2d)>;
  using key_fun      = fun_t<void(window&, win_key_data)>;
  using cursorp_fun  = fun_t<void(window&, dvec2)>;
  using cursore_fun  = fun_t<void(window&, bool)>;
  using scroll_fun   = fun_t<void(window&, dvec2)>;
  using mouse_fun    = fun_t<void(window&, win_button_data)>;
  using char_fun     = fun_t<void(window&, uint32)>;

public:
  window(window_t handle, context_api ctx_api, win_attrib attrib) noexcept;

public:
  [[nodiscard]] static win_expect<window> create(const win_params& params);

  static context_gl_params make_gl_params(window_t handle) noexcept;
  static context_gl_params make_gl_params(const window& win) noexcept;

public:
  template<typename F>
  requires(std::invocable<F, window&, extent2d>)
  window& set_viewport_callback(F&& fun) {
    _callbacks.viewport = std::forward<F>(fun);
    return *this;
  }

  template<typename F>
  requires(std::invocable<F, window&, win_key_data>)
  window& set_key_press_callback(F&& fun) {
    _callbacks.keypress = std::forward<F>(fun);
    return *this;
  }

  template<typename F>
  requires(std::invocable<F, window&, win_button_data>)
  window& set_button_press_callback(F&& fun) {
    _callbacks.buttpress = std::forward<F>(fun);
    return *this;
  }

  template<typename F>
  requires(std::invocable<F, window&, dvec2>)
  window& set_cursor_pos_callback(F&& fun) {
    _callbacks.cursorpos = std::forward<F>(fun);
    return *this;
  }

  template<typename F>
  requires(std::invocable<F, window&, bool>)
  window& set_cursor_enter_callback(F&& fun) {
    _callbacks.cursorenter = std::forward<F>(fun);
    return *this;
  }

  template<typename F>
  requires(std::invocable<F, window&, dvec2>)
  window& set_scroll_callback(F&& fun) {
    _callbacks.scroll = std::forward<F>(fun);
    return *this;
  }

  template<typename F>
  requires(std::invocable<F, window&, uint32>)
  window& set_char_input_callback(F&& fun) {
    _callbacks.char_input = std::forward<F>(fun);
    return *this;
  }

  void title(const std::string& title);
  void close();
  void poll_events();
  void set_mouse_state(win_mouse_state state);
  void attribs(win_attrib attrib);

public:
  window_t get() const { return _handle; }
  context_api renderer() const { return _ctx_api; }
  win_attrib attribs() const { return _attrib; }

  bool should_close() const;
  win_action poll_key(win_key key) const;
  win_action poll_button(win_button button) const;
  extent2d win_size() const;
  extent2d fb_size() const;

private:
  void _destroy();

private:
  window_t _handle;
  context_api _ctx_api;
  win_attrib _attrib;
  struct {
    viewport_fun viewport;
    key_fun keypress;
    mouse_fun buttpress;
    cursorp_fun cursorpos;
    cursore_fun cursorenter;
    scroll_fun scroll;
    char_fun char_input;
  } _callbacks;

public:
  NTF_DECLARE_MOVE_ONLY(window);
};

} // namespace ntf::render

namespace shogle = ntf::render;
namespace ntfr = ntf::render;
