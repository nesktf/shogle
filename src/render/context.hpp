#pragma once

#include "./render.hpp"
#include "./window.hpp"

#include "../stl/expected.hpp"
#include "../stl/optional.hpp"
#include "../stl/arena.hpp"

#define SHOGLE_DEFINE_RENDER_TYPE_MOVE(_type) \
void reset() noexcept {  \
  if (has_value()) { \
    _ctx->destroy(_handle); \
    _ctx.reset(); \
    _handle = {}; \
  } \
} \
~_type() noexcept { reset(); } \
_type(const _type&) = delete; \
_type& operator=(const _type&) = delete; \
_type(_type&& other) noexcept : \
  _ctx{std::move(other._ctx)}, _handle{std::move(other._handle)} { \
  if (other.has_value()) { \
    other._ctx.reset(); \
    other._handle = {}; \
  } \
} \
_type& operator=(_type&& other) noexcept { \
  if (std::addressof(other) == this) { \
    return *this; \
  } \
  reset(); \
  _ctx = std::move(other._ctx); \
  _handle = std::move(other._handle); \
  other._ctx.reset(); \
  other._handle = {}; \
  return *this; \
}

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

using r_error = ::ntf::error<void>;

template<typename T>
using r_expected = ::ntf::expected<T, r_error>;

class r_context {
public:
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
    std::vector<weak_ref<texture_binding_t>> textures;
    std::vector<weak_ref<uniform_descriptor_t>> uniforms;
    uint32 count;
    uint32 offset;
    uint32 instances;
  };

  struct draw_list_t {
    color4 color;
    uvec4 viewport;
    r_clear_flag clear;
    std::vector<weak_ref<draw_command_t>> cmds;
  };
  using command_map = std::unordered_map<r_framebuffer_handle, r_context::draw_list_t>;

  struct vertex_attrib_t {
    uint32 binding;
    size_t stride;
    std::vector<r_attrib_descriptor> descriptors;
  };

  struct buff_store_t {
    r_buffer_type type;
    r_buffer_flag flags;
    size_t size;
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

  struct shader_store_t {
    r_shader_type type;
  };

  using uniform_map = std::unordered_map<std::string, r_uniform>;
  struct pipeline_store_t {
    r_stages_flag stages;

    std::unique_ptr<vertex_attrib_t> layout;

    r_primitive primitive;
    r_polygon_mode poly_mode;
    r_front_face front_face;
    r_cull_mode cull_mode;

    r_pipeline_test tests;
    optional<r_compare_op> depth_ops;
    optional<r_compare_op> stencil_ops;

    uniform_map uniforms;
  };

  struct fb_store_t {
    uvec2 extent;
    // uvec4 viewport;
    // color4 clear_color;

    r_test_buffer_flag buffers;
    optional<r_test_buffer_format> buffer_format;

    std::vector<r_framebuffer_attachment> attachments;
    optional<r_texture_format> color_buffer_format;
  };

  struct ctx_meta_t {
    r_api api;
    std::string name_str;
    std::string vendor_str;
    std::string version_str;
    uint32 tex_max_layers;
    uint32 tex_max_extent;
    uint32 tex_max_extent3d;
  };

public:
  static constexpr r_framebuffer_handle DEFAULT_FRAMEBUFFER{};

public:
  r_context() = default;

public:
  void init(r_window& win, const r_context_params& params = {}) noexcept;

public:
  void start_frame() noexcept;
  void end_frame() noexcept;
  void device_wait() noexcept;

public:
  [[nodiscard]] r_expected<r_buffer_handle> create_buffer(const r_buffer_descriptor& desc);
  [[nodiscard]] r_buffer_handle create_buffer(const r_buffer_descriptor& desc,
                                              unchecked_t);
  void destroy(r_buffer_handle buff) noexcept;
  [[nodiscard]] r_expected<void> update(r_buffer_handle buff, const r_buffer_data& desc);

  [[nodiscard]] r_buffer_type query(r_buffer_handle buff, r_query_type_t) const;
  [[nodiscard]] size_t query(r_buffer_handle buff, r_query_size_t) const;

public:
  [[nodiscard]] r_expected<r_texture_handle> create_texture(const r_texture_descriptor& desc);
  [[nodiscard]] r_texture_handle create_texture(const r_texture_descriptor& desc,
                                                unchecked_t);
  void destroy(r_texture_handle tex) noexcept;
  [[nodiscard]] r_expected<void> update(r_texture_handle tex, const r_texture_data& update);

  [[nodiscard]] r_texture_type query(r_texture_handle tex, r_query_type_t) const;
  [[nodiscard]] r_texture_format query(r_texture_handle tex, r_query_format_t) const;
  [[nodiscard]] r_texture_sampler query(r_texture_handle tex, r_query_sampler_t) const;
  [[nodiscard]] r_texture_address query(r_texture_handle tex, r_query_addressing_t) const;
  [[nodiscard]] uvec3 query(r_texture_handle tex, r_query_extent_t) const;
  [[nodiscard]] uint32 query(r_texture_handle tex, r_query_layers_t) const;
  [[nodiscard]] uint32 query(r_texture_handle tex, r_query_levels_t) const;

public:
  [[nodiscard]] auto create_framebuffer(const r_framebuffer_descriptor& desc)
                                                               -> r_expected<r_framebuffer_handle>;
  void destroy(r_framebuffer_handle fbo) noexcept;

public:
  [[nodiscard]] r_expected<r_shader_handle> create_shader(const r_shader_descriptor& desc);
  void destroy(r_shader_handle shader) noexcept;

  [[nodiscard]] r_shader_type query(r_shader_handle shader, r_query_type_t) const;

public:
  [[nodiscard]] r_pipeline_handle create_pipeline(const r_pipeline_descriptor& desc);
  void destroy(r_pipeline_handle pipeline);

  [[nodiscard]] r_stages_flag query(r_pipeline_handle pipeline, r_query_stages_t) const;
  [[nodiscard]] r_uniform query(r_pipeline_handle pipeline, r_query_uniform_t,
                                std::string_view name) const;

public:
  void framebuffer_viewport(r_framebuffer_handle fbo, uvec4 vp) {
    NTF_ASSERT(_draw_lists.find(fbo) != _draw_lists.end());
    _draw_lists.at(fbo).viewport = vp;
  }
  void framebuffer_viewport(uvec4 vp) { framebuffer_viewport(DEFAULT_FRAMEBUFFER, vp); }

  void framebuffer_color(r_framebuffer_handle fbo, color4 color) {
    NTF_ASSERT(_draw_lists.find(fbo) != _draw_lists.end());
    _draw_lists.at(fbo).color = color;
  }
  void framebuffer_color(color4 color) { framebuffer_color(DEFAULT_FRAMEBUFFER, color); }

  void framebuffer_clear(r_framebuffer_handle fbo, r_clear_flag flags) {
    NTF_ASSERT(_draw_lists.find(fbo) != _draw_lists.end());
    _draw_lists.at(fbo).clear = flags;
  }
  void framebuffer_clear(r_clear_flag flags) { framebuffer_clear(DEFAULT_FRAMEBUFFER, flags); }

  void bind_texture(r_texture_handle texture, uint32 index) {
    auto* ptr = _frame_arena.allocate<texture_binding_t>(1);
    ptr->handle = texture;
    ptr->index = index;
    _d_cmd.textures.emplace_back(ptr);
  }

  void bind_framebuffer(r_framebuffer_handle fbo) {
    NTF_ASSERT(_draw_lists.find(fbo) != _draw_lists.end());
    _d_list = _draw_lists.at(fbo);
  }

  void bind_vertex_buffer(r_buffer_handle buffer) {
    _d_cmd.vertex_buffer = buffer;
  }

  void bind_index_buffer(r_buffer_handle buffer) {
    _d_cmd.index_buffer = buffer;
  }

  void bind_pipeline(r_pipeline_handle pipeline) {
    _d_cmd.pipeline = pipeline;
  }

  void draw_opts(r_draw_opts opts) {
    _d_cmd.count = opts.count;
    _d_cmd.offset = opts.offset;
    _d_cmd.instances = opts.instances;
  }

  template<typename T>
  requires(r_attrib_traits<T>::is_attrib)
  void push_uniform(r_uniform location, const T& data) {
    NTF_ASSERT(location);
    auto* desc = _frame_arena.allocate<uniform_descriptor_t>(1);
    desc->location = location;
    desc->type = r_attrib_traits<T>::tag;
    auto* data_ptr = _frame_arena.template allocate<T>(1);
    std::construct_at(data_ptr, data);
    desc->data = data_ptr;
    _d_cmd.uniforms.emplace_back(desc);
  }

  void submit() {
    auto* cmd = _frame_arena.allocate<draw_command_t>(1);
    weak_ref<draw_command_t> ref = std::construct_at(cmd, std::move(_d_cmd));
    _d_list->cmds.emplace_back(ref);
    _d_cmd = {};
  }

public:
  bool valid() const { return bool{_ctx}; }
  explicit operator bool() const { return valid(); }

  r_api render_api() const { return _ctx_meta.api; }
  std::string_view name_str() const;
  r_window& win() { return *_win; }

private:
  ntf::mem_arena _frame_arena;
  weak_ref<r_window> _win;
  std::unique_ptr<r_platform_context> _ctx;
  ctx_meta_t _ctx_meta;

  std::unordered_map<r_buffer_handle, buff_store_t> _buffers;
  std::unordered_map<r_texture_handle, tex_store_t> _textures;
  std::unordered_map<r_framebuffer_handle, fb_store_t> _framebuffers;
  std::unordered_map<r_shader_handle, shader_store_t> _shaders;
  std::unordered_map<r_pipeline_handle, pipeline_store_t> _pipelines;

  std::unordered_map<r_framebuffer_handle, draw_list_t> _draw_lists;
  weak_ref<draw_list_t> _d_list;
  draw_command_t _d_cmd{};

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(r_context);
};

struct r_platform_context {
  virtual ~r_platform_context() = default;
  virtual r_context::ctx_meta_t query_meta() const = 0;

  virtual r_buffer_handle create_buffer(const r_buffer_descriptor& desc) = 0;
  virtual void update_buffer(r_buffer_handle buf, const r_buffer_data& data) = 0;
  virtual void destroy_buffer(r_buffer_handle buf) = 0;

  virtual r_texture_handle create_texture(const r_texture_descriptor& desc) = 0;
  virtual void update_texture(r_texture_handle tex, const r_texture_data& desc) = 0;
  virtual void destroy_texture(r_texture_handle tex) = 0;

  virtual r_shader_handle create_shader(const r_shader_descriptor& desc) = 0;
  virtual void destroy_shader(r_shader_handle shader) = 0;

  virtual r_pipeline_handle create_pipeline(const r_pipeline_descriptor& desc,
                                            weak_ref<r_context::vertex_attrib_t> attrib,
                                            r_context::uniform_map& uniforms) = 0;
  virtual void destroy_pipeline(r_pipeline_handle pipeline) = 0;

  virtual r_framebuffer_handle create_framebuffer(const r_framebuffer_descriptor& desc) = 0;
  virtual void destroy_framebuffer(r_framebuffer_handle fb) = 0;

  virtual void submit(const r_context::command_map&) = 0;

  virtual void device_wait() noexcept {}
};


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
