#pragma once

#include "../context.hpp"

#define SHOGLE_DECLARE_RENDER_HANDLE(_name) \
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

namespace ntf {

using r_handle_value = uint32;
constexpr r_handle_value r_handle_tombstone = std::numeric_limits<r_handle_value>::max();

template<typename T>
struct r_handle_hash {
  r_handle_hash() noexcept = default;
  size_t operator()(const T& handle) const noexcept {
    return std::hash<r_handle_value>{}(static_cast<r_handle_value>(handle));
  }
};

SHOGLE_DECLARE_RENDER_HANDLE(r_platform_buffer);
SHOGLE_DECLARE_RENDER_HANDLE(r_platform_texture);
SHOGLE_DECLARE_RENDER_HANDLE(r_platform_shader);
SHOGLE_DECLARE_RENDER_HANDLE(r_platform_pipeline);
SHOGLE_DECLARE_RENDER_HANDLE(r_platform_fbo);

constexpr r_platform_fbo DEFAULT_FBO_HANDLE{r_handle_tombstone};

template<typename K, typename T>
using handle_map = std::unordered_map<K, T, r_handle_hash<K>>;

struct vertex_layout {
  uint32 binding;
  size_t stride;
  std::vector<r_attrib_descriptor> descriptors;
};

using uniform_map = std::unordered_map<std::string, r_uniform>;

struct uniform_descriptor {
  r_attrib_type type;
  r_uniform location;
  const void* data;
};

struct texture_binding {
  r_texture handle;
  uint32 index;
};

struct draw_command {
  r_pipeline pipeline;
  std::vector<weak_ref<texture_binding>> textures;
  std::vector<weak_ref<uniform_descriptor>> uniforms;
  std::vector<weak_ref<r_buffer_binding>> buffers;
  uint32 count;
  uint32 offset;
  uint32 instances;
  std::function<void()> on_render;
};

struct draw_list {
  color4 color;
  uvec4 viewport;
  r_clear_flag clear;
  std::vector<weak_ref<draw_command>> cmds;
};

using command_map = handle_map<r_platform_fbo, draw_list>;

struct r_platform_meta {
  renderer_api api;
  std::string name_str;
  std::string vendor_str;
  std::string version_str;
  uint32 tex_max_layers;
  uint32 tex_max_extent;
  uint32 tex_max_extent3d;
};

struct r_buff_desc {
  r_buffer_type type;
  r_buffer_flag flags;
  size_t size;
  weak_cref<r_buffer_data> initial_data;
};

struct r_buff_data {
  const void* data;
  size_t len;
  size_t offset;
};

struct r_buff_mapping {
  size_t len;
  size_t offset;
};

struct r_tex_desc {
  r_texture_type type;
  r_texture_format format;
  extent3d extent;
  uint32 layers;
  uint32 levels;
  cspan<r_image_data> initial_data;
  bool gen_mipmaps;
  r_texture_sampler sampler;
  r_texture_address addressing;
};

struct r_tex_image_data {
  const void* texels;
  r_texture_format format;
  r_image_alignment alignment;
  extent3d extent;
  extent2d offset;
  uint32 layer;
  uint32 level;
  bool regen_mips;
};

struct r_tex_opts {
  r_texture_sampler sampler;
  r_texture_address addressing;
};

struct r_shad_desc {
  r_shader_type type;
  cspan<std::string_view> source;
};

struct r_pip_desc {
  std::unique_ptr<vertex_layout> layout;
  weak_ref<uniform_map> uniforms;

  cspan<r_platform_shader> stages;
  r_primitive primitive;
  r_polygon_mode poly_mode;

  weak_cref<r_stencil_test_opts> stencil_test;
  weak_cref<r_depth_test_opts> depth_test;
  weak_cref<r_scissor_test_opts> scissor_test;
  weak_cref<r_face_cull_opts> face_culling;
  weak_cref<r_blend_opts> blending;
};

struct r_pip_opts {
  weak_cref<r_stencil_test_opts> stencil_test;
  weak_cref<r_depth_test_opts> depth_test;
  weak_cref<r_scissor_test_opts> scissor_test;
  weak_cref<r_face_cull_opts> face_culling;
  weak_cref<r_blend_opts> blending;
};

struct r_fbo_att {
  r_platform_texture texture;
  uint32 layer;
  uint32 level;
};

struct r_fbo_desc {
  extent2d extent;
  uvec4 viewport;
  r_test_buffer test_buffer;
  std::variant<cspan<r_fbo_att>, r_texture_format> attachments;
};


struct r_platform_context {
  virtual ~r_platform_context() = default;
  virtual r_platform_meta get_meta() = 0;

  virtual r_platform_buffer create_buffer(const r_buff_desc& desc) = 0;
  virtual void update_buffer(r_platform_buffer buf, const r_buff_data& data) = 0;
  virtual void* map_buffer(r_platform_buffer buf, const r_buff_mapping& mapping) = 0;
  virtual void unmap_buffer(r_platform_buffer buf, void* ptr) = 0;
  virtual void destroy_buffer(r_platform_buffer buf) noexcept = 0;

  virtual r_platform_texture create_texture(const r_tex_desc& desc) = 0;
  virtual void update_texture_image(r_platform_texture tex, const r_tex_image_data& image) = 0;
  virtual void update_texture_options(r_platform_texture tex, const r_tex_opts& opts) = 0;
  virtual void destroy_texture(r_platform_texture tex) noexcept = 0;

  virtual r_platform_shader create_shader(const r_shad_desc& desc) = 0;
  virtual void destroy_shader(r_platform_shader shader) noexcept = 0;

  virtual r_platform_pipeline create_pipeline(const r_pip_desc& desc) = 0;
  virtual void update_pipeline_options(r_pipeline pip, const r_pip_opts& opts) = 0;
  virtual void destroy_pipeline(r_platform_pipeline pipeline) noexcept = 0;

  virtual r_platform_fbo create_framebuffer(const r_fbo_desc& desc) = 0;
  virtual void destroy_framebuffer(r_platform_fbo fb) noexcept = 0;

  virtual void submit(const command_map& cmds) = 0;

  virtual void device_wait() noexcept {}
  virtual void swap_buffers() = 0;
};

r_platform_buffer r_buffer_get_handle(r_buffer buff);
r_platform_texture r_texture_get_handle(r_texture tex);
r_platform_fbo r_framebuffer_get_handle(r_framebuffer fbo);
r_platform_shader r_shader_get_handle(r_shader shader);
r_platform_pipeline r_pipeline_get_handle(r_pipeline pip);

} // namespace ntf

#undef SHOGLE_DECLARE_RENDER_HANDLE
