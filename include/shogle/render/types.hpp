#pragma once

#include <shogle/math/vector.hpp>

#if defined(SHOGLE_EXPOSE_GLFW) && SHOGLE_EXPOSE_GLFW
#include <glad/glad.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif


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

NTF_DECLARE_OPAQUE_HANDLE(context_t);
NTF_DECLARE_OPAQUE_HANDLE(window_t);
NTF_DECLARE_OPAQUE_HANDLE(texture_t);
NTF_DECLARE_OPAQUE_HANDLE(buffer_t);
NTF_DECLARE_OPAQUE_HANDLE(shader_t);
NTF_DECLARE_OPAQUE_HANDLE(pipeline_t);
NTF_DECLARE_OPAQUE_HANDLE(framebuffer_t);
NTF_DECLARE_OPAQUE_HANDLE(uniform_t);

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


using color3 = vec3;
using color4 = vec4;

using extent1d = uint32;
using extent2d = uvec2;
using extent3d = uvec3;

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

constexpr uint8 IMAGE_DEPTH_CHANNELS_MASK = 0b00000111;
constexpr uint8 IMAGE_DEPTH_NORMALIZE_BIT = 0b10000000;
constexpr uint8 IMAGE_DEPTH_NONLINEAR_BIT = 0b01000000;

enum class context_api {
  none = 0,
  opengl,
  vulkan,
  software,
};

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

enum class fbo_buffer : uint8 {
  none = 0,
  depth16u,
  depth24u,
  depth32f,
  depth24u_stencil8u,
  depth32f_stencil8u,
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

namespace shogle = ntf::render;
namespace ntfr = ntf::render;
