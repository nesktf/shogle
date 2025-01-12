#pragma once

#include "./render.hpp"
#include "./window.hpp"

#include "../stl/optional.hpp"
#include "../stl/arena.hpp"

namespace ntf {

struct r_platform_context;

struct r_context_params {
  optional<r_api> use_api;
};

NTF_DECLARE_TAG_TYPE(r_query_size);
NTF_DECLARE_TAG_TYPE(r_query_type);
NTF_DECLARE_TAG_TYPE(r_query_format);
NTF_DECLARE_TAG_TYPE(r_query_sampler);
NTF_DECLARE_TAG_TYPE(r_query_addressing);
NTF_DECLARE_TAG_TYPE(r_query_extent);
NTF_DECLARE_TAG_TYPE(r_query_layers);
NTF_DECLARE_TAG_TYPE(r_query_levels);
NTF_DECLARE_TAG_TYPE(r_query_stages);
NTF_DECLARE_TAG_TYPE(r_query_uniform);

class r_context {
public:
  static constexpr r_framebuffer_handle DEFAULT_FRAMEBUFFER{};

public:
  struct context_str_t {
    std::string_view name;
    std::string_view vendor;
    std::string_view version;
  };

  struct uniform_descriptor_t {
    r_attrib_type type;
    r_uniform location;
    const void* data;
  };

  struct texture_binding_t {
    r_texture_handle handle;
    uint32 index;
  };

  struct draw_command_t {
    r_buffer_handle vertex_buffer;
    r_buffer_handle index_buffer;
    r_pipeline_handle pipeline;
    const texture_binding_t* textures;
    uint32 texture_count;
    const uniform_descriptor_t* uniforms;
    uint32 uniform_count;
    r_draw_opts opts;
  };
  using command_queue = std::vector<weak_ref<draw_command_t>>;

  struct buffer_create_t {
    r_buffer_type type;
    const void* data;
    size_t size;
  };

  struct buffer_update_t {
    const void* data;
    size_t size;
    size_t offset;
  };

  struct buff_store_t {
    r_buffer_type type;
    size_t size;
  };

  struct tex_create_t {
    r_texture_type type;
    r_texture_format format;
    void const* const* texels;
    uvec3 extent;
    uint32 layers;
    uint32 levels;
    bool gen_mipmaps;
    r_texture_sampler sampler;
    r_texture_address addressing;
  };

  struct tex_update_t {
    r_texture_format format;
    const void* texels;
    uvec3 offset;
    uint32 layer;
    uint32 level;
    bool genmips;
    optional<r_texture_sampler> sampler;
    optional<r_texture_address> addressing;
  };

  struct tex_store_t {
    std::atomic<uint32> refcount;
    r_texture_type type;
    r_texture_format format;
    uvec3 extent;
    uint32 layers;
    uint32 levels;
    r_texture_address addressing;
    r_texture_sampler sampler;
  };

  struct shader_create_t {
    r_shader_type type;
    std::string_view src;
  };

  struct shader_store_t {
    r_shader_type type;
  };

  struct vertex_attributes_t {
    std::vector<r_attrib_descriptor> desc;
    size_t stride;
  };

  struct pipeline_create_t {
    const r_shader_handle* shaders;
    uint32 shader_count;
    weak_ref<vertex_attributes_t> layout;
    r_primitive primitive;
  };

  struct pipeline_store_t {
    vertex_attributes_t layout;
    r_stages_flag stages;
    // std::unordered_map<r_uniform, std::string> uniforms;
    r_primitive primitive;
    r_polygon_mode poly_mode;
    r_front_face front_face;
    r_cull_mode cull_mode;
    r_pipeline_test tests;
    r_compare_op depth_ops;
    r_compare_op stencil_ops;
  };

  struct fb_create_t {
    uvec2 extent;
    r_test_buffer_flag buffers;
    r_texture_format buffer_format;
    r_texture_format color_buffer_format;
    const r_framebuffer_att* attachments;
    uint32 attachment_count;
  };

  struct fb_update_t {
    uvec4 viewport;
    color4 color;
    r_clear_flag flags;
  };

  struct fb_store_t {
    uvec4 viewport;
    color4 color;
    // r_clear_flag flags;
    std::vector<weak_ref<draw_command_t>> cmds;
    std::vector<r_texture_handle> attachments;
  };

public:
  r_context() = default;

public:
  void init(r_window& win, const r_context_params& params = {}) noexcept;

public:
  void start_frame() noexcept;
  void end_frame() noexcept;
  void device_wait() noexcept;

public:
  [[nodiscard]] r_buffer_handle create_buffer(const r_buffer_descriptor& desc);
  void destroy(r_buffer_handle buff);
  void update(r_buffer_handle buff,
              const void* data, size_t size, size_t offset);
  [[nodiscard]] r_buffer_type query(r_buffer_handle buff, r_query_type_t) const;
  [[nodiscard]] size_t query(r_buffer_handle buff, r_query_size_t) const;

  [[nodiscard]] r_texture_handle create_texture(const r_texture_descriptor& desc);
  void destroy(r_texture_handle tex);
  void update(r_texture_handle tex, r_texture_sampler sampler);
  void update(r_texture_handle tex, r_texture_address addressing);
  void update(r_texture_handle tex,
              const void* texels, r_texture_format format, uvec3 offset,
              uint32 layer, uint32 level, bool genmips = false);
  [[nodiscard]] r_texture_type query(r_texture_handle tex, r_query_type_t) const;
  [[nodiscard]] r_texture_format query(r_texture_handle tex, r_query_format_t) const;
  [[nodiscard]] r_texture_sampler query(r_texture_handle tex, r_query_sampler_t) const;
  [[nodiscard]] r_texture_address query(r_texture_handle tex, r_query_addressing_t) const;
  [[nodiscard]] uvec3 query(r_texture_handle tex, r_query_extent_t) const;
  [[nodiscard]] uint32 query(r_texture_handle tex, r_query_layers_t) const;
  [[nodiscard]] uint32 query(r_texture_handle tex, r_query_levels_t) const;

  [[nodiscard]] r_framebuffer_handle create_framebuffer(const r_framebuffer_descriptor& desc);
  void destroy(r_framebuffer_handle fbo);

  [[nodiscard]] r_shader_handle create_shader(const r_shader_descriptor& desc);
  void destroy(r_shader_handle shader);
  [[nodiscard]] r_shader_type query(r_shader_handle shader, r_query_type_t) const;

  [[nodiscard]] r_pipeline_handle create_pipeline(const r_pipeline_descriptor& desc);
  void destroy(r_pipeline_handle pipeline);
  [[nodiscard]] r_stages_flag query(r_pipeline_handle pipeline, r_query_stages_t) const;
  [[nodiscard]] r_uniform query(r_pipeline_handle pipeline, r_query_uniform_t,
                                std::string_view name) const;

public:
  void framebuffer_viewport(r_framebuffer_handle fbo, uvec4 vp);
  void framebuffer_viewport(uvec4 vp) { framebuffer_viewport(DEFAULT_FRAMEBUFFER, vp); }

  void framebuffer_color(r_framebuffer_handle fbo, color4 color);
  void framebuffer_color(color4 color) { framebuffer_color(DEFAULT_FRAMEBUFFER, color); }

  void framebuffer_clear(r_framebuffer_handle fbo, r_clear_flag flags);
  void framebuffer_clear(r_clear_flag flags) { framebuffer_clear(DEFAULT_FRAMEBUFFER, flags); }

  void bind_texture(r_texture_handle texture, uint32 index);
  void bind_framebuffer(r_framebuffer_handle fbo);
  void bind_vertex_buffer(r_buffer_handle buffer);
  void bind_index_buffer(r_buffer_handle buffer);
  void bind_pipeline(r_pipeline_handle pipeline);
  void draw_opts(r_draw_opts opts);

  void submit();

  template<typename T>
  requires(r_attrib_traits<T>::is_attrib)
  void push_uniform(r_uniform location, const T& data) {
    auto* ptr = _frame_arena.allocate<T>(1);
    _uniforms.emplace_back(uniform_descriptor_t{
      .type = r_attrib_traits<T>::tag,
      .location = location,
      .data = std::construct_at(ptr, data),
    });
  }

public:
  bool valid() const { return bool{_ctx}; }
  explicit operator bool() const { return valid(); }

  r_api render_api() const { return _ctx_api; }
  std::string_view name_str() const;
  r_window& win() { return *_win; }

private:
  weak_ref<r_window> _win;
  std::unique_ptr<r_platform_context> _ctx;
  r_api _ctx_api;

  std::unordered_map<r_buffer_handle, buff_store_t> _buffers;
  std::unordered_map<r_texture_handle, tex_store_t> _textures;
  std::unordered_map<r_framebuffer_handle, fb_store_t> _framebuffers;
  std::unordered_map<r_shader_handle, shader_store_t> _shaders;
  std::unordered_map<r_pipeline_handle, pipeline_store_t> _pipelines;

  ntf::mem_arena _frame_arena;
  std::vector<uniform_descriptor_t> _uniforms;

  std::unordered_map<r_framebuffer_handle, std::vector<weak_ref<draw_command_t>>> _cmds;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(r_context);
};

struct r_platform_context {
  virtual ~r_platform_context() = default;
  virtual r_api api_type() const = 0;
  virtual r_context::context_str_t ctx_str() const = 0;

  virtual r_buffer_handle create_buffer(const r_context::buffer_create_t& data) = 0;
  virtual void update_buffer(r_buffer_handle buf, const r_context::buffer_update_t& data) = 0;
  virtual void destroy_buffer(r_buffer_handle buf) = 0;

  virtual r_texture_handle create_texture(const r_context::tex_create_t& data) = 0;
  virtual void update_texture(r_texture_handle tex, const r_context::tex_update_t& data) = 0;
  virtual void destroy_texture(r_texture_handle tex) = 0;

  virtual r_shader_handle create_shader(const r_context::shader_create_t& data) = 0;
  virtual void destroy_shader(r_shader_handle shader) = 0;

  virtual r_pipeline_handle create_pipeline(const r_context::pipeline_create_t& data) = 0;
  virtual r_uniform pipeline_uniform(r_pipeline_handle pipeline, std::string_view name) = 0;
  virtual void destroy_pipeline(r_pipeline_handle pipeline) = 0;

  virtual r_framebuffer_handle create_framebuffer(const r_context::fb_create_t& data) = 0;
  virtual void update_framebuffer(r_framebuffer_handle fb, const r_context::fb_update_t& data) = 0;
  virtual void destroy_framebuffer(r_framebuffer_handle fb) = 0;

  virtual void submit(r_framebuffer_handle fb, const r_context::command_queue& cmds) = 0;

  virtual void device_wait() noexcept {}
};


// class r_texture_view {
// public:
//   r_texture_view() :
//     _ctx{}, _handle{} {}
//   r_texture_view(r_context& ctx, r_texture_handle handle) :
//     _ctx{ctx}, _handle{handle} {}
//
// public:
//   r_texture_view& sampler(r_texture_sampler sampler) {
//     _ctx->update(_handle, sampler);
//     return *this;
//   }
//
//   r_texture_view& addressing(r_texture_address addressing) {
//     _ctx->update(_handle, addressing);
//     return *this;
//   }
//
//   r_texture_view& update(const void* data, size_t size, size_t offset, r_texture_format format,
//                          uint32 layer, uint32 mipmap) {
//     _ctx->update_data(_handle, data, size, offset, format, layer, mipmap);
//     return *this;
//   }
//
// public:
//   [[nodiscard]] r_texture_type type() const {
//     return _ctx->query(_handle, r_query_type);
//   }
//   [[nodiscard]] r_texture_sampler sampler() const {
//     return _ctx->query(_handle, r_query_sampler);
//   }
//   [[nodiscard]] r_texture_address addressing() const {
//     return _ctx->query(_handle, r_query_addressing);
//   }
//   [[nodiscard]] r_texture_format format() const {
//     return _ctx->query(_handle, r_query_format);
//   }
//
//   [[nodiscard]] uvec3 dim() const {
//     return _ctx->query(_handle, r_query_extent);
//   }
//   [[nodiscard]] uvec2 dim2d() const {
//     auto d = dim(); return uvec2{d.x, d.y};
//   }
//
//   [[nodiscard]] uint32 layers() const {
//     return _ctx->query(_handle, r_query_layers);
//   }
//   [[nodiscard]] uint32 mipmaps() const {
//     return _ctx->query(_handle, r_query_levels);
//   }
//
//   [[nodiscard]] bool is_cubemap() const {
//     return type() == r_texture_type::cubemap;
//   }
//   [[nodiscard]] bool is_array() const {
//     return !is_cubemap() && layers() > 1;
//   }
//   [[nodiscard]] bool has_mipmaps() const {
//     return mipmaps() > 0;
//   }
//
// public:
//   [[nodiscard]] bool valid() const { return _ctx && _handle; }
//   explicit operator bool() const { return valid(); }
//   operator r_texture_handle() const { return _handle; }
//
// protected:
//   weak_ref<r_context> _ctx;
//   r_texture_handle _handle;
// };


template<typename F>
concept delta_time_func = std::invocable<F, float64>; // f(dt) -> void

template<typename F>
concept fixed_render_func = std::invocable<F, float64, float64>; // f(dt, alpha) -> void

template<typename F>
concept fixed_update_func = std::invocable<F, uint32>; // f(ups) -> void


template<typename T>
concept has_render_member = requires(T t) {
  { t.on_render(float64{}) } -> std::convertible_to<void>;
};

template<typename T>
concept has_fixed_render_member = requires(T t) {
  { t.on_render(float64{}, float64{}) } -> std::convertible_to<void>;
};

template<typename T>
concept has_fixed_update_member = requires(T t) {
  { t.on_fixed_update(uint32{}) } -> std::convertible_to<void>;
};


template<typename T>
concept fixed_loop_object = 
  (fixed_render_func<T> && fixed_update_func<T>) ||
  (has_fixed_render_member<T> && has_fixed_update_member<T>);

template<typename T>
concept nonfixed_loop_object = delta_time_func<T> || has_render_member<T>;


template<nonfixed_loop_object LoopObj>
void shogle_render_loop(r_context& ctx, LoopObj&& obj) {
  using namespace std::literals;

  using clock = std::chrono::steady_clock;
  using duration = clock::duration;
  using time_point = std::chrono::time_point<clock, duration>;

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Main loop started");

  time_point last_time = clock::now();
  r_window& window = ctx.win();
  while (!window.should_close()) {
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
    last_time = start_time;

    float64 dt {std::chrono::duration<float64>{elapsed_time}/1s};

    window.poll_events();

    ctx.start_frame();
    if constexpr (has_render_member<LoopObj>) {
      obj.on_render(dt);
    } else {
      obj(dt);
    }
    ctx.end_frame();
  }
  ctx.device_wait();

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Main loop exit");
}

template<fixed_loop_object LoopObj>
void shogle_render_loop(r_context& ctx, uint32 ups, LoopObj&& obj) {
  using namespace std::literals;

  const auto fixed_elapsed_time =
    std::chrono::duration<float64>{std::chrono::microseconds{1000000/ups}};

  using clock = std::chrono::steady_clock;
  using duration = decltype(clock::duration{} + fixed_elapsed_time);
  using time_point = std::chrono::time_point<clock, duration>;

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Fixed main loop started at {} ups", ups);

  time_point last_time = clock::now();
  r_window& window = ctx.win();
  duration lag = 0s;
  while (!window.should_close()) {
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
    last_time = start_time;
    lag += elapsed_time;

    float64 dt {std::chrono::duration<float64>{elapsed_time}/1s};
    float64 alpha {std::chrono::duration<float64>{lag}/fixed_elapsed_time};

    window.poll_events();

    ctx.start_frame();

    while (lag >= fixed_elapsed_time) {
      if constexpr (has_fixed_update_member<LoopObj>) {
        obj.on_fixed_update(ups);
      } else {
        obj(ups);
      }
      lag -= fixed_elapsed_time;
    }

    if constexpr (has_fixed_render_member<LoopObj>) {
      obj.on_render(dt, alpha);
    } else {
      obj(dt, alpha);
    }

    ctx.end_frame();
  }
  ctx.device_wait();

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Main loop exit");
}

template<fixed_render_func RFunc, fixed_update_func UFunc>
void shogle_render_loop(r_context& ctx, uint32 ups, RFunc&& render, UFunc&& fixed_update) {
  using namespace std::literals;

  const auto fixed_elapsed_time =
    std::chrono::duration<float64>{std::chrono::microseconds{1000000/ups}};

  using clock = std::chrono::steady_clock;
  using duration = decltype(clock::duration{} + fixed_elapsed_time);
  using time_point = std::chrono::time_point<clock, duration>;

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Fixed main loop started at {} ups", ups);

  time_point last_time = clock::now();
  duration lag = 0s;
  r_window& window = ctx.win();
  while (!window.should_close()) {
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
    last_time = start_time;
    lag += elapsed_time;

    float64 dt {std::chrono::duration<float64>{elapsed_time}/1s};
    float64 alpha {std::chrono::duration<float64>{lag}/fixed_elapsed_time};

    window.poll_events();

    ctx.start_frame();

    while (lag >= fixed_elapsed_time) {
      fixed_update(ups);
      lag -= fixed_elapsed_time;
    }

    render(dt, alpha);

    ctx.end_frame();
  }
  ctx.device_wait();

  SHOGLE_LOG(debug, "[ntf::shogle_main_loop] Main loop exit");
}

} // namespace ntf
