#pragma once

#include "./platform_macros.hpp"

#include "../stl/ptr.hpp"
#include "../stl/optional.hpp"
#include "../stl/expected.hpp"
#include "../stl/function.hpp"

#include "../math/matrix.hpp"

namespace ntf {

NTF_DECLARE_OPAQUE_HANDLE(r_context);

using r_error = ::ntf::error<void>;

template<typename T>
using r_expected = expected<T, r_error>;

struct r_allocator {
  void* user_ptr;
  void* (*mem_alloc)(void* user_ptr, size_t size, size_t alignment);
  void  (*mem_free)(void* user_ptr, void* mem);
  void* (*mem_scratch_alloc)(void* user_ptr, size_t size, size_t alignment);
  void  (*mem_scratch_free)(void* user_ptr, void* mem);
};

NTF_DECLARE_OPAQUE_HANDLE(r_texture);

enum class r_texture_type : uint8 {
  texture1d = 0,
  texture2d,
  texture3d,
  cubemap,
};

enum class r_texture_format : uint8 {
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

enum class r_image_alignment : uint8 {
  bytes1 = 1,
  bytes2 = 2,
  bytes4 = 4,
  bytes8 = 8,
};

struct r_image_data {
  const void* texels;
  r_texture_format format;
  r_image_alignment alignment;

  extent3d extent;
  extent3d offset;

  uint32 layer;
  uint32 level;
};

struct r_texture_descriptor {
  r_texture_type type;
  r_texture_format format;

  extent3d extent;
  uint32 layers;
  uint32 levels;

  cspan<r_image_data> images;
  bool gen_mipmaps{false};
  r_texture_sampler sampler;
  r_texture_address addressing;
};

struct r_texture_data {
  cspan<r_image_data> images;
  bool gen_mipmaps{false};
  optional<r_texture_sampler> sampler;
  optional<r_texture_address> addressing;
};

struct r_texture_binding {
  r_texture texture;
  uint32 location;
};

r_expected<r_texture> r_create_texture(r_context ctx, const r_texture_descriptor& desc);
r_texture r_create_texture(unchecked_t, r_context ctx, const r_texture_descriptor& desc);
void r_destroy_texture(r_texture tex);

r_expected<void> r_texture_upload(r_texture tex, const r_texture_data& data);
void r_texture_upload(unchecked_t, r_texture tex, const r_texture_data& data);
r_expected<void> r_texture_upload(r_texture tex, cspan<r_image_data> images, bool gen_mips);
void r_texture_upload(unchecked_t, r_texture tex, cspan<r_image_data> images, bool gen_mips);
r_expected<void> r_texture_set_sampler(r_texture tex, r_texture_sampler sampler);
void r_texture_set_sampler(unchecked_t, r_texture tex, r_texture_sampler sampler);
r_expected<void> r_texture_set_addressing(r_texture tex, r_texture_address adressing);
void r_texture_set_addressing(unchecked_t, r_texture tex, r_texture_address addressing);

r_texture_type r_texture_get_type(r_texture tex);
r_texture_format r_texture_get_format(r_texture tex);
r_texture_sampler r_texture_get_sampler(r_texture tex);
r_texture_address r_texture_get_addressing(r_texture tex);
extent3d r_texture_get_extent(r_texture tex);
uint32 r_texture_get_layers(r_texture tex);
uint32 r_texture_get_levels(r_texture tex); 
r_context r_texture_get_ctx(r_texture tex);


NTF_DECLARE_OPAQUE_HANDLE(r_buffer);

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
  read_mappable     = 1 << 1,
  write_mappable    = 1 << 2,
  rw_mappable       = (1<<1) | (1<<2),
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(r_buffer_flag);

struct r_buffer_data {
  const void* data;
  size_t size;
  size_t offset;
};

struct r_buffer_descriptor {
  r_buffer_type type;
  r_buffer_flag flags;
  size_t size;

  weak_cref<r_buffer_data> data;
};

struct r_buffer_binding {
  r_buffer buffer;
  r_buffer_type type;
  optional<uint32> location;
};

r_expected<r_buffer> r_create_buffer(r_context ctx, const r_buffer_descriptor& desc);
r_buffer r_create_buffer(unchecked_t, r_context ctx, const r_buffer_descriptor& desc);
void r_destroy_buffer(r_buffer buffer);

r_expected<void> r_buffer_upload(r_buffer buff, size_t offset, size_t len, const void* data);
void r_buffer_upload(unchecked_t, r_buffer buff, size_t offset, size_t len, const void* data);
r_expected<void*> r_buffer_map(r_buffer buff, size_t offset, size_t len);
void* r_buffer_map(unchecked_t, r_buffer buff, size_t offset, size_t len);
void r_buffer_unmap(r_buffer buff, void* mapped);

r_buffer_type r_buffer_get_type(r_buffer buff);
size_t r_buffer_get_size(r_buffer buff);
r_context r_buffer_get_ctx(r_buffer buff);


NTF_DECLARE_OPAQUE_HANDLE(r_shader);

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

struct r_shader_descriptor {
  r_shader_type type;
  cspan<std::string_view> source;
};

r_expected<r_shader> r_create_shader(r_context ctx, const r_shader_descriptor& desc);
r_shader r_create_shader(unchecked_t, r_context ctx, const r_shader_descriptor& desc);
void r_destroy_shader(r_shader shader);

r_shader_type r_shader_get_type(r_shader shader);
r_context r_shader_get_ctx(r_shader shader);


NTF_DECLARE_OPAQUE_HANDLE(r_pipeline);

enum class r_attrib_type : uint32 {
  f32, vec2,  vec3,  vec4,  mat3,  mat4,
  f64, dvec2, dvec3, dvec4, dmat3, dmat4,
  i32, ivec2, ivec3, ivec4,
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

enum class r_test_func : uint8 {
  never = 0,
  always,
  less,
  greater,
  equal,
  lequal,
  gequal,
  nequal,
};

enum class r_stencil_op : uint8 {
  keep = 0,
  set_zero,
  replace,
  incr,
  incr_wrap,
  decr,
  decr_wrap,
  invert,
};

enum class r_cull_face : uint8 {
  clockwise = 0,
  counter_clockwise,
};

enum class r_cull_mode : uint8 {
  front = 0,
  back,
  front_back,
};

enum class r_blend_mode : uint8 {
  min = 0,
  max,
  add,
  subs,
  rev_subs,
};

enum class r_blend_factor : uint8 {
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

struct r_blend_opts {
  r_blend_mode mode;
  r_blend_factor src_factor;
  r_blend_factor dst_factor;
  color4 color;
  bool dynamic;
};

struct r_stencil_rule {
  r_stencil_op on_stencil_fail;
  r_stencil_op on_depth_fail;
  r_stencil_op on_pass;
};

struct r_stencil_test_opts {
  r_test_func test_func;
  int32 ref_value;
  uint32 ref_mask;
  r_stencil_rule stencil_rule;
  optional<r_stencil_rule> alt_rule;
  uint32 stencil_mask;
  optional<uint32> alt_mask;
  bool dynamic;
};

struct r_depth_test_opts {
  r_test_func test_func;
  double near_bound;
  double far_bound;
  bool dynamic;
};

struct r_scissor_test_opts {
  extent2d pos;
  extent2d size;
  bool dynamic;
};

struct r_face_cull_opts {
  r_cull_mode mode;
  r_cull_face front_face;
  bool dynamic;
};

struct r_attrib_descriptor {
  r_attrib_type type;
  uint32 binding;
  uint32 location;
  size_t offset;
};

struct r_pipeline_descriptor {
  uint32 attrib_binding;
  size_t attrib_stride;
  cspan<r_attrib_descriptor> attribs;
  cspan<r_shader> stages;
  r_primitive primitive;
  r_polygon_mode poly_mode;

  weak_cref<r_stencil_test_opts> stencil_test;
  weak_cref<r_depth_test_opts> depth_test;
  weak_cref<r_scissor_test_opts> scissor_test;
  weak_cref<r_face_cull_opts> face_culling;
  weak_cref<r_blend_opts> blending;
};

// SHOGLE_DECLARE_RENDER_HANDLE(r_uniform);

// struct r_uniform_descriptor {
//   r_attrib_type type;
//   r_uniform location;
//   const void* data;
// };

using r_uniform = uint32;

struct r_push_constant {
  r_uniform location;
  const void* data;
  r_attrib_type type;
  size_t alignment;
};

r_expected<r_pipeline> r_create_pipeline(r_context ctx, const r_pipeline_descriptor& desc);
r_pipeline r_create_pipeline(unchecked_t, r_context ctx, const r_pipeline_descriptor& desc);
void r_destroy_pipeline(r_pipeline pip);

r_stages_flag r_pipeline_get_stages(r_pipeline pip);
optional<r_uniform> r_pipeline_get_uniform(r_pipeline pip, std::string_view name);
r_uniform r_pipeline_get_uniform(unchecked_t, r_pipeline pip, std::string_view name);

r_context r_pipeline_get_ctx(r_pipeline pip);


NTF_DECLARE_OPAQUE_HANDLE(r_framebuffer);

enum class r_test_buffer : uint8 {
  no_buffer = 0,
  depth16u,
  depth24u,
  depth32f,
  depth24u_stencil8u,
  depth32f_stencil8u,
};

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

struct r_framebuffer_attachment {
  r_texture handle;
  uint32 layer;
  uint32 level;
};

struct r_framebuffer_descriptor {
  uvec2 extent;
  uvec4 viewport;
  color4 clear_color;
  r_clear_flag clear_flags;
  r_test_buffer test_buffer;
  std::variant<cspan<r_framebuffer_attachment>, r_texture_format> attachments;
};

r_expected<r_framebuffer> r_create_framebuffer(r_context ctx,
                                               const r_framebuffer_descriptor& desc);
                                               
r_framebuffer r_create_framebuffer(unchecked_t, r_context ctx,
                                   const r_framebuffer_descriptor& desc);
                                   
void r_destroy_framebuffer(r_framebuffer fbo);

void r_framebuffer_set_clear(r_framebuffer fbo, r_clear_flag flags);
void r_framebuffer_set_viewport(r_framebuffer fbo, const uvec4& vp);
void r_framebuffer_set_color(r_framebuffer fbo, const color4& color);

r_clear_flag r_framebuffer_get_clear(r_framebuffer fbo);
color4 r_framebuffer_get_color(r_framebuffer fbo);
uvec4 r_framebuffer_get_viewport(r_framebuffer fbo);

r_framebuffer r_get_default_framebuffer(r_context ctx);
r_context r_framebuffer_get_ctx(r_framebuffer fbo);

struct r_draw_opts {
  uint32 count;
  uint32 offset;
  uint32 instances;
  uint32 sort_group;
};

struct r_draw_command {
  r_framebuffer target;
  r_pipeline pipeline;
  cspan<r_buffer_binding> buffers;
  cspan<r_texture_binding> textures;
  cspan<r_push_constant> uniforms;
  r_draw_opts draw_opts;
  function_view<void(r_context)> on_render;
};

struct r_context_params {
  win_handle window;
  renderer_api api;
  uint32 swap_interval;

  uvec4 fb_viewport;
  r_clear_flag fb_clear;
  color4 fb_color;

  weak_cref<r_allocator> alloc; // Placeholder!!!
};

r_expected<r_context> r_create_context(const r_context_params& params);
void r_destroy_context(r_context ctx);

void r_start_frame(r_context ctx);
void r_end_frame(r_context ctx);
void r_device_wait(r_context ctx);
void r_submit_command(r_context ctx, const r_draw_command& cmd);

} // namespace ntf
