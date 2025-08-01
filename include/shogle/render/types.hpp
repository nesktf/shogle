#pragma once

#include <shogle/core.hpp>
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

#define SHOGLE_DECLARE_ATTRIB_TRAIT(_type, _tag, _underlying, _get_ptr) \
static_assert(std::is_trivial_v<_type>, NTF_STRINGIFY(_type) " is not trivial!!!"); \
template<> \
struct attribute_traits<_type> { \
  static constexpr bool is_specialized = true; \
  using underlying_type = _underlying; \
  static constexpr shogle::attribute_type tag = _tag; \
  static constexpr size_t size = attribute_size(tag); \
  static constexpr u32 dim = attribute_dim(tag); \
  static const _underlying* value_ptr(const _type& obj) noexcept { \
    return _get_ptr; \
  } \
}

namespace shogle {

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

using color3 = vec3;
using color4 = vec4;

using extent1d = u32;
using extent2d = uvec2;
using extent3d = uvec3;

enum class image_format : u32 {
  r8u,    r16u,    r32f,
  rg8u,   rg16u,   rg32f,
  rgb8u,  rgb16u,  rgb32f,
  rgba8u, rgba16u, rgba32f,
};

enum class context_api {
  none = 0,
  opengl,
  vulkan,
  software,
};

enum class primitive_mode : u8 {
  triangles = 0,
  triangle_strip,
  triangle_fan,
  lines,
  line_strip,
  points,
};

enum class polygon_mode : u8 {
  fill = 0,
  point,
  line,
};

enum class test_func : u8 {
  never = 0,
  always,
  less,
  greater,
  equal,
  lequal,
  gequal,
  nequal,
};

enum class stencil_op : u8 {
  keep = 0,
  set_zero,
  replace,
  incr,
  incr_wrap,
  decr,
  decr_wrap,
  invert,
};

enum class cull_face : u8 {
  clockwise = 0,
  counter_clockwise,
};

enum class cull_mode : u8 {
  front = 0,
  back,
  front_back,
};

enum class blend_mode : u8 {
  min = 0,
  max,
  add,
  subs,
  rev_subs,
};

enum class blend_factor : u8 {
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
  u32 mask;
};

struct stencil_test_opts {
  stencil_func func;
  stencil_rule rule;
  u32 mask;
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
  weak_ptr<const stencil_test_opts> stencil_test;
  weak_ptr<const depth_test_opts> depth_test;
  weak_ptr<const scissor_test_opts> scissor_test;
  weak_ptr<const face_cull_opts> face_culling;
  weak_ptr<const blend_opts> blending;
};

enum class clear_flag : u8 {
  none          = 0,
  color         = 1 << 0,
  depth         = 1 << 1,
  stencil       = 1 << 2,
  color_depth   = (1<<0) | (1<<1),
  color_stencil = (1<<0) | (1<<2),
  all           = (1<<0) | (1<<1) | (1<<2),
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(clear_flag);

enum class fbo_buffer : u8 {
  none = 0,
  depth16u,
  depth24u,
  depth32f,
  depth24u_stencil8u,
  depth32f_stencil8u,
};

enum class attribute_type : u32 {
  f32, vec2,  vec3,  vec4,  mat3,  mat4,
  f64, dvec2, dvec3, dvec4, dmat3, dmat4,
  i32, ivec2, ivec3, ivec4,
};

struct attribute_binding {
  attribute_type type;
  u32 location;
  size_t offset;
  size_t stride;
};

// For uniforms (who cares about double precision matrices??? Use an uniform buffer!!!!!)
using attribute_data = ntf::inplace_trivial<sizeof(mat4), alignof(mat4)>;

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

namespace meta {

template<typename T>
struct attribute_traits {
  static constexpr bool is_specialized = false;
};

constexpr inline size_t attribute_size(shogle::attribute_type type) noexcept {
  constexpr size_t int_sz = sizeof(int32);
  constexpr size_t f32_sz = sizeof(f32);
  constexpr size_t f64_sz = sizeof(f64);

  switch (type) {
    case attribute_type::i32:   return int_sz;
    case attribute_type::ivec2: return 2*int_sz;
    case attribute_type::ivec3: return 3*int_sz;
    case attribute_type::ivec4: return 4*int_sz;

    case attribute_type::f32:   return f32_sz;
    case attribute_type::vec2:  return 2*f32_sz;
    case attribute_type::vec3:  return 3*f32_sz;
    case attribute_type::vec4:  return 4*f32_sz;
    case attribute_type::mat3:  return 9*f32_sz;
    case attribute_type::mat4:  return 16*f32_sz;

    case attribute_type::f64:   return f64_sz;
    case attribute_type::dvec2: return 2*f64_sz;
    case attribute_type::dvec3: return 3*f64_sz;
    case attribute_type::dvec4: return 4*f64_sz;
    case attribute_type::dmat3: return 9*f64_sz;
    case attribute_type::dmat4: return 16*f64_sz;
  };

  return 0;
};

constexpr inline u32 attribute_dim(shogle::attribute_type type) noexcept {
  switch (type) {
    case attribute_type::i32:   [[fallthrough]];
    case attribute_type::f32:   [[fallthrough]];
    case attribute_type::f64:   return 1;

    case attribute_type::vec2:  [[fallthrough]];
    case attribute_type::ivec2: [[fallthrough]];
    case attribute_type::dvec2: return 2;

    case attribute_type::vec3:  [[fallthrough]];
    case attribute_type::ivec3: [[fallthrough]];
    case attribute_type::dvec3: return 3;

    case attribute_type::vec4:  [[fallthrough]];
    case attribute_type::ivec4: [[fallthrough]];
    case attribute_type::dvec4: return 4;

    case attribute_type::mat3:  [[fallthrough]];
    case attribute_type::dmat3: return 9;

    case attribute_type::mat4:  [[fallthrough]];
    case attribute_type::dmat4: return 16;
  };

  return 0;
}

SHOGLE_DECLARE_ATTRIB_TRAIT(f32, shogle::attribute_type::f32, f32, &obj);
SHOGLE_DECLARE_ATTRIB_TRAIT(vec2, shogle::attribute_type::vec2, f32, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(vec3, shogle::attribute_type::vec3, f32, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(vec4, shogle::attribute_type::vec4, f32, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(mat3, shogle::attribute_type::mat3, f32, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(mat4, shogle::attribute_type::mat4, f32, glm::value_ptr(obj));

SHOGLE_DECLARE_ATTRIB_TRAIT(f64, shogle::attribute_type::f64, f64, &obj);
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec2, shogle::attribute_type::dvec2, f64, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec3, shogle::attribute_type::dvec3, f64, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec4, shogle::attribute_type::dvec4, f64, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(dmat3, shogle::attribute_type::dmat3, f64, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(dmat4, shogle::attribute_type::dmat4, f64, glm::value_ptr(obj));

SHOGLE_DECLARE_ATTRIB_TRAIT(int32, shogle::attribute_type::i32, int32, &obj);
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec2, shogle::attribute_type::ivec2, int32, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec3, shogle::attribute_type::ivec3, int32, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec4, shogle::attribute_type::ivec4, int32, glm::value_ptr(obj));

template<typename T>
concept attribute_type = attribute_traits<T>::is_specialized;


template<typename T>
struct image_depth_traits {
  static constexpr bool is_specialized = false;
};

NTF_DECLARE_TAG_TYPE(image_depth_u8);
template<>
struct image_depth_traits<u8> {
  static constexpr bool is_specialized = true;

  using tag_type = image_depth_u8_t;
  static constexpr tag_type tag = image_depth_u8;

  static constexpr bool is_signed = false;
  static constexpr bool is_floating = false;
  static constexpr std::string_view name = "8 bit unsigned";

  static constexpr ntf::optional<image_format> parse_channels(u32 nch) noexcept {
    switch (nch) {
      case 1u: return image_format::r8u;
      case 2u: return image_format::rg8u;
      case 3u: return image_format::rgb8u;
      case 4u: return image_format::rgba8u;
    }
    return ntf::nullopt;
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

  static constexpr ntf::optional<image_format> parse_channels(u32 nch) noexcept {
    switch (nch) {
      case 1u: return image_format::r16u;
      case 2u: return image_format::rg16u;
      case 3u: return image_format::rgb16u;
      case 4u: return image_format::rgba16u;
    }
    return ntf::nullopt;
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

  static constexpr ntf::optional<image_format> parse_channels(u32 nch) noexcept {
    switch (nch) {
      case 1u: return image_format::r32f;
      case 2u: return image_format::rg32f;
      case 3u: return image_format::rgb32f;
      case 4u: return image_format::rgba32f;
    }
    return ntf::nullopt;
  }
};

template<typename T>
concept image_depth_type = image_depth_traits<T>::is_specialized;

template<typename T>
struct image_dim_traits {
  static constexpr bool is_specialized = false;
};

template<>
struct image_dim_traits<shogle::extent1d> {
  static constexpr bool is_specialized = true;
  static constexpr bool allows_arrays = true;

  template<image_depth_type T>
  constexpr static size_t image_stride(shogle::extent1d extent) noexcept {
    return static_cast<size_t>(extent)*sizeof(T);
  }
  constexpr static shogle::extent3d extent_cast(shogle::extent1d extent) noexcept {
    return {static_cast<u32>(extent), 1u, 1u};
  }
  constexpr static shogle::extent3d offset_cast(shogle::extent1d offset, u32 layer = 0u) noexcept {
    return {static_cast<u32>(offset), layer, 0u};
  }
};

template<>
struct image_dim_traits<shogle::extent2d> {
  static constexpr bool is_specialized = true;
  static constexpr bool allows_arrays = true;

  template<image_depth_type T>
  constexpr static size_t image_stride(shogle::extent2d extent) noexcept {
    return static_cast<size_t>(extent.x*extent.y)*sizeof(T);
  }
  constexpr static shogle::extent3d extent_cast(shogle::extent2d extent) noexcept {
    return {static_cast<u32>(extent.x), static_cast<u32>(extent.y), 1u};
  }
  constexpr static shogle::extent3d offset_cast(shogle::extent2d offset, u32 layer = 0u) noexcept {
    return {static_cast<u32>(offset.x), static_cast<u32>(offset.y), layer};
  }
};

template<>
struct image_dim_traits<shogle::extent3d> {
  static constexpr bool is_specialized = true;
  static constexpr bool allows_arrays = false;

  template<image_depth_type T>
  constexpr static size_t image_stride(shogle::extent3d extent) noexcept {
    return static_cast<size_t>(extent.x*extent.y*extent.y)*sizeof(T);
  }
  constexpr static shogle::extent3d extent_cast(shogle::extent3d extent) noexcept {
    return extent;
  }
  constexpr static shogle::extent3d offset_cast(shogle::extent3d offset) noexcept {
    return offset;
  }
};

template<typename T>
concept image_dim_type = image_dim_traits<T>::is_specialized;

template<typename T>
concept image_array_dim_type = image_dim_type<T> && image_dim_traits<T>::allows_arrays;

} // namespace meta

} // namespace shogle

#undef SHOGLE_DECLARE_ATTRIB_TRAIT
