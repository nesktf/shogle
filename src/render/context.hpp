#pragma once

#include "./types.hpp"
#include "./window.hpp"

#include "../stl/expected.hpp"
#include "../stl/optional.hpp"
#include "../stl/arena.hpp"

namespace ntf {

struct r_platform_context;

struct r_context_params {
  weak_ref<r_window> window;
  optional<r_api> use_api;
  uint32 swap_interval;
};

struct r_context_data {
public:
  struct uniform_descriptor {
    r_attrib_type type;
    r_uniform location;
    const void* data;
  };

  struct texture_binding {
    r_texture_handle handle;
    uint32 index;
  };

  struct draw_command {
    r_buffer_handle vertex_buffer;
    r_buffer_handle index_buffer;
    r_pipeline_handle pipeline;
    std::vector<weak_ref<texture_binding>> textures;
    std::vector<weak_ref<uniform_descriptor>> uniforms;
    uint32 count;
    uint32 offset;
    uint32 instances;
  };

  struct draw_list {
    color4 color;
    uvec4 viewport;
    r_clear_flag clear;
    std::vector<weak_ref<draw_command>> cmds;
  };
  using command_map = std::unordered_map<r_framebuffer_handle, draw_list>;

  using uniform_map = std::unordered_map<std::string, r_uniform>;
  struct vertex_layout {
    uint32 binding;
    size_t stride;
    std::vector<r_attrib_descriptor> descriptors;
  };

  struct ctx_meta {
    r_api api;
    std::string name_str;
    std::string vendor_str;
    std::string version_str;
    uint32 tex_max_layers;
    uint32 tex_max_extent;
    uint32 tex_max_extent3d;
  };

  struct buffer_store {
    buffer_store(const r_buffer_descriptor& desc) noexcept;

    r_buffer_type type;
    r_buffer_flag flags;
    size_t size;
  };

  struct texture_store {
    texture_store(const r_texture_descriptor& desc) noexcept;

    std::atomic<uint32> refcount;
    r_texture_type type;
    r_texture_format format;
    uvec3 extent;
    uint32 levels;
    uint32 layers;
    r_texture_address addressing;
    r_texture_sampler sampler;
  };

  struct shader_store {
    shader_store(const r_shader_descriptor& desc) noexcept;

    r_shader_type type;
  };

  struct framebuffer_store {
    framebuffer_store(const r_framebuffer_descriptor& desc) noexcept;

    uvec2 extent;
    r_test_buffer_flag buffers;
    optional<r_test_buffer_format> buffer_format;
    std::vector<r_framebuffer_attachment> attachments;
    optional<r_texture_format> color_buffer_format;
  };

  struct pipeline_store {
    pipeline_store(const r_pipeline_descriptor& desc,
                   std::unique_ptr<vertex_layout> layout,
                   uniform_map uniforms,
                   r_stages_flag stages_) noexcept;
    r_stages_flag stages;

    r_primitive primitive;
    r_polygon_mode poly_mode;
    r_front_face front_face;
    r_cull_mode cull_mode;

    r_pipeline_test tests;
    optional<r_compare_op> depth_ops;
    optional<r_compare_op> stencil_ops;

    std::unique_ptr<vertex_layout> layout;

    uniform_map uniforms;
  };

public:
  r_context_data(win_handle_t win_,
                 std::unique_ptr<r_platform_context> ctx_,
                 command_map map_) noexcept;

  ~r_context_data() noexcept;

public:
  ntf::mem_arena frame_arena;
  win_handle_t win;
  std::unique_ptr<r_platform_context> platform;

  std::unordered_map<r_buffer_handle, buffer_store> buffers;
  std::unordered_map<r_texture_handle, texture_store> textures;
  std::unordered_map<r_framebuffer_handle, framebuffer_store> framebuffers;
  std::unordered_map<r_shader_handle, shader_store> shaders;
  std::unordered_map<r_pipeline_handle, pipeline_store> pipelines;

  std::unordered_map<r_framebuffer_handle, draw_list> draw_lists;
  weak_ref<draw_list> d_list;
  draw_command d_cmd;
};

struct r_platform_context {
  virtual ~r_platform_context() = default;
  virtual r_context_data::ctx_meta query_meta() const = 0;

  virtual r_buffer_handle create_buffer(const r_buffer_descriptor& desc) = 0;
  virtual void update_buffer(r_buffer_handle buf, const r_buffer_data& data) = 0;
  virtual void destroy_buffer(r_buffer_handle buf) noexcept = 0;

  virtual r_texture_handle create_texture(const r_texture_descriptor& desc) = 0;
  virtual void update_texture(r_texture_handle tex, const r_texture_data& desc) = 0;
  virtual void destroy_texture(r_texture_handle tex) noexcept = 0;

  virtual r_shader_handle create_shader(const r_shader_descriptor& desc) = 0;
  virtual void destroy_shader(r_shader_handle shader) noexcept = 0;

  virtual r_pipeline_handle create_pipeline(const r_pipeline_descriptor& desc,
                                            weak_ref<r_context_data::vertex_layout> layout,
                                            r_context_data::uniform_map& uniforms) = 0;
  virtual void destroy_pipeline(r_pipeline_handle pipeline) noexcept = 0;

  virtual r_framebuffer_handle create_framebuffer(const r_framebuffer_descriptor& desc) = 0;
  virtual void destroy_framebuffer(r_framebuffer_handle fb) noexcept = 0;

  virtual void submit(win_handle_t win, const r_context_data::command_map&) = 0;

  virtual void device_wait() noexcept {}
  virtual void swap_buffers(win_handle_t win) = 0;
};

class r_context_view {
public:
  static constexpr r_framebuffer_handle DEFAULT_FRAMEBUFFER{};

public:
  r_context_view(weak_ref<r_context_data> data) noexcept :
    _data{data} {}

public:
  void start_frame() noexcept;
  void end_frame() noexcept;
  void device_wait() noexcept;

  void bind_texture(r_texture_handle texture, uint32 index) noexcept;
  void bind_framebuffer(r_framebuffer_handle fbo) noexcept;
  void bind_vertex_buffer(r_buffer_handle buffer) noexcept;
  void bind_index_buffer(r_buffer_handle buffer) noexcept;
  void bind_pipeline(r_pipeline_handle pipeline) noexcept;
  void draw_opts(r_draw_opts opts) noexcept;
  void submit() noexcept;
  void submit(const r_draw_command& cmd) noexcept;

  template<typename T>
  requires(r_attrib_traits<T>::is_attrib)
  void push_uniform(r_uniform location, const T& data) {
    NTF_ASSERT(location);
    auto* desc = _data->frame_arena.allocate<r_context_data::uniform_descriptor>(1);
    auto* data_ptr = _data->frame_arena.template allocate<T>(1);
    std::construct_at(data_ptr, data);

    desc->location = location;
    desc->type = r_attrib_traits<T>::tag;
    desc->data = data_ptr;
    _data->d_cmd.uniforms.emplace_back(desc);
  }

public:
  [[nodiscard]] r_expected<r_buffer_handle> buffer_create(
    const r_buffer_descriptor& desc
  ) noexcept;

  [[nodiscard]] r_buffer_handle buffer_create(
    unchecked_t,
    const r_buffer_descriptor& desc
  );

  void destroy(r_buffer_handle buff) noexcept;

  r_expected<void> buffer_update(
    r_buffer_handle buff,
    const r_buffer_data& desc
  ) noexcept;

  void buffer_update(
    unchecked_t,
    r_buffer_handle buff,
    const r_buffer_data& desc
  );

  [[nodiscard]] r_buffer_type buffer_type(r_buffer_handle buff) const;
  [[nodiscard]] size_t buffer_size(r_buffer_handle buff) const;

public:
  [[nodiscard]] r_expected<r_texture_handle> texture_create(
    const r_texture_descriptor& desc
  ) noexcept;

  [[nodiscard]] r_texture_handle texture_create(
    unchecked_t,
    const r_texture_descriptor& desc
  );

  void destroy(r_texture_handle tex) noexcept;

  r_expected<void> texture_update(
    r_texture_handle tex,
    const r_texture_data& data
  ) noexcept;

  void texture_update(
    unchecked_t,
    r_texture_handle tex,
    const r_texture_data& data
  );

  r_expected<void> texture_update(
    r_texture_handle tex,
    span_view<r_image_data> images,
    bool gen_mipmaps
  ) noexcept;

  void texture_update(
    unchecked_t,
    r_texture_handle tex,
    span_view<r_image_data> images,
    bool gen_mipmaps
  );

  r_expected<void> texture_sampler(
    r_texture_handle tex,
    r_texture_sampler sampler
  ) noexcept;

  void texture_sampler(
    unchecked_t,
    r_texture_handle tex,
    r_texture_sampler sampler
  );

  r_expected<void> texture_addressing(
    r_texture_handle tex,
    r_texture_address addressing
  ) noexcept;

  void texture_addressing(
    unchecked_t,
    r_texture_handle tex,
    r_texture_address addressing
  );

  [[nodiscard]] r_texture_type texture_type(r_texture_handle tex) const;
  [[nodiscard]] r_texture_format texture_format(r_texture_handle tex) const;
  [[nodiscard]] r_texture_sampler texture_sampler(r_texture_handle tex) const;
  [[nodiscard]] r_texture_address texture_addressing(r_texture_handle tex) const;
  [[nodiscard]] uvec3 texture_extent(r_texture_handle tex) const;
  [[nodiscard]] uint32 texture_layers(r_texture_handle tex) const;
  [[nodiscard]] uint32 texture_levels(r_texture_handle tex) const;

public:
  [[nodiscard]] r_expected<r_framebuffer_handle> framebuffer_create(
    const r_framebuffer_descriptor& desc
  ) noexcept;

  [[nodiscard]] r_framebuffer_handle framebuffer_create(
    unchecked_t,
    const r_framebuffer_descriptor& desc
  );

  void destroy(r_framebuffer_handle fbo) noexcept;

  void framebuffer_clear(r_framebuffer_handle fbo, r_clear_flag flags);
  void framebuffer_viewport(r_framebuffer_handle fbo, uvec4 vp);
  void framebuffer_color(r_framebuffer_handle fbo, color4 color);

  [[nodiscard]] r_clear_flag framebuffer_clear(r_framebuffer_handle fbo) const;
  [[nodiscard]] uvec4 framebuffer_viewport(r_framebuffer_handle fbo) const;
  [[nodiscard]] color4 framebuffer_color(r_framebuffer_handle fbo) const;

public:
  [[nodiscard]] r_expected<r_shader_handle> shader_create(
    const r_shader_descriptor& desc
  ) noexcept;

  [[nodiscard]] r_shader_handle shader_create(
    unchecked_t,
    const r_shader_descriptor& desc
  );

  void destroy(r_shader_handle shader) noexcept;

  [[nodiscard]] r_shader_type shader_type(r_shader_handle shader) const;

public:
  [[nodiscard]] r_expected<r_pipeline_handle> pipeline_create(
    const r_pipeline_descriptor& desc
  ) noexcept;

  [[nodiscard]] r_pipeline_handle pipeline_create(unchecked_t,
    const r_pipeline_descriptor& desc
                                                  );

  void destroy(r_pipeline_handle pipeline) noexcept;

  [[nodiscard]] r_stages_flag pipeline_stages(r_pipeline_handle pipeline) const;

  [[nodiscard]] optional<r_uniform> pipeline_uniform(
    r_pipeline_handle pipeline,
   std::string_view name) const noexcept;

  [[nodiscard]] r_uniform pipeline_uniform(
    unchecked_t,
    r_pipeline_handle pipeline,
    std::string_view name) const;

private:
  void _populate_attachments(
    r_context_data::framebuffer_store& fb,
    const r_framebuffer_descriptor& desc,
    r_framebuffer_handle handle
  );

  std::unique_ptr<r_context_data::vertex_layout> _copy_pipeline_layout(
    const r_pipeline_descriptor& desc
  );

public:
  // r_api render_api() const { return _ctx->query_meta().api; }
  std::string_view name_str() const;

protected:
  weak_ref<r_context_data> _data;
};

class r_context : public std::unique_ptr<r_context_data>, public r_context_view {
private:
  using uptr_base = std::unique_ptr<r_context_data>;

public:
  r_context(uptr_base data) noexcept;

public:
  static r_expected<r_context> create(const r_context_params& params) noexcept;
  static r_context create(unchecked_t, const r_context_params& params);

public:
  ~r_context() noexcept;

  r_context(const r_context&) = delete;
  r_context& operator=(const r_context&) = delete;

  r_context(r_context&&) = default;
  r_context& operator=(r_context&& other) noexcept {
    r_context_view::operator=(std::move(other));
    uptr_base::operator=(std::move(other));
    return *this;
  }
};

} // namespace ntf
