#pragma once

#include "../core.hpp"
#include "../math/vector.hpp"

#include <ntfstl/ptr.hpp>
#include <ntfstl/expected.hpp>
#include <ntfstl/optional.hpp>
#include <ntfstl/function.hpp>
#include <ntfstl/memory_pool.hpp>

namespace ntf::render {

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

namespace ntfr = ntf::render;
