#pragma once

#include "./render.hpp"
#include "./window.hpp"

#include "../stl/optional.hpp"
#include "../stl/arena.hpp"

#define SHOGLE_DECLARE_CONTEXT_HANDLE(_name) \
class _name { \
public: \
  _name() : _handle{r_handle_tombstone} {} \
  explicit _name(r_handle_value handle) : _handle{handle} {} \
public: \
  r_handle_value value() const { return _handle; } \
  operator r_handle_value() const { return value(); } \
  bool valid() const { return _handle == r_handle_tombstone; } \
  explicit operator bool() const { return valid(); } \
private: \
  r_handle_value _handle; \
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
NTF_DECLARE_TAG_TYPE(r_query_dim);
NTF_DECLARE_TAG_TYPE(r_query_layer);
NTF_DECLARE_TAG_TYPE(r_query_mipmaps);
NTF_DECLARE_TAG_TYPE(r_query_stages);

class r_context {
public:
  // Internal handles, used in the actual platform context
  SHOGLE_DECLARE_CONTEXT_HANDLE(buff_handle);
  SHOGLE_DECLARE_CONTEXT_HANDLE(tex_handle);
  SHOGLE_DECLARE_CONTEXT_HANDLE(shad_handle);
  SHOGLE_DECLARE_CONTEXT_HANDLE(pip_handle);
  SHOGLE_DECLARE_CONTEXT_HANDLE(fb_handle);

  using command_queue = std::vector<weak_ref<r_draw_cmd>>;

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

  struct tex_create_t {
    r_texture_type type;
    r_texture_format format;
    void const* const* data;
    uvec3 dim;
    uint32 array_size;
    uint32 mipmaps;
    r_texture_sampler sampler;
    r_texture_address addressing;
  };

  struct tex_param_t {
    optional<r_texture_sampler> sampler;
    optional<r_texture_address> addressing;
  };

  struct tex_data_t {
    r_texture_format format;
    const void* data;
    uvec3 offset;
    uint32 index;
    uint32 level;
    bool gen_mips;
  };

  struct fbuff_create_t {
    uint32 w, h;
    r_clear_flag buffers;
    r_texture_format format;

    const r_texture_handle* attachments;
    uint32 attachment_count;
    const uint32* attachment_levels;
  };

  struct shader_create_t {
    std::string_view src;
    r_texture_type type;
  };

  struct pipeline_create_t {
    const r_shader_handle* shaders;
    uint32 shader_count;
    weak_ref<r_attributes> layout;
  };

private:
  struct buff_handle_data_t {
    buff_handle handle;

    size_t size;
  };

  struct tex_handle_data_t {
    tex_handle handle;

    r_texture_type type;
    r_texture_format format;
    r_texture_sampler sampler;
    r_texture_address addressing;
    uvec3 dim;
    uint32 layers;
    uint32 mipmaps;
  };

  struct fb_handle_data_t {
    fb_handle handle;
  };

  struct shad_handle_data_t {
    shad_handle handle;
  };

  struct pip_handle_data_t {
    pip_handle handle;

    uint32 layout_index;
  };

  template<typename T, typename H>
  class res_container {
  public:
    res_container() = default;

  public:
    template<typename Fun>
    void clear(Fun&& f) {
      for (auto& res : _res) {
        f(res);
      }
      _res.clear();
      _free = {};
    }

    H acquire() {
      if (!_free.empty()) {
        H pos = H{_free.front()};
        _free.pop();
        return pos;
      }
      _res.emplace_back(T{});
      return H{_res.size()-1};
    }

    void push(H pos) {
      NTF_ASSERT(validate(pos));
      _free.push(pos.value());
    }

    T& get(H pos) {
      NTF_ASSERT(validate(pos));
      return _res[pos.value()];
    }

    const T& get(H pos) const {
      NTF_ASSERT(validate(pos));
      return _res[pos.value()];
    }

    bool validate(H pos) const {
      return pos.value() < _res.size();
    }

  private:
    std::vector<T> _res;
    std::queue<r_handle_value> _free;
  };

  struct uniform_descriptor_t {
    r_uniform location;
    r_attrib_type type;
    const void* data;
  };

public:
  void init(r_window& win, const r_context_params& params = {}) noexcept;

public:
  void start_frame() noexcept;
  void end_frame() noexcept;
  void device_wait() noexcept;

public:
  [[nodiscard]] r_buffer_handle create_buffer(const r_buffer_descriptor& desc);
  void destroy(r_buffer_handle buff);
  void update_data(r_buffer_handle buff,
                   const void* data, size_t size, size_t offset);
  [[nodiscard]] r_buffer_type query(r_buffer_handle buff, r_query_type_t) const;
  [[nodiscard]] size_t query(r_buffer_handle buff, r_query_size_t) const;

  [[nodiscard]] r_texture_handle create_texture(const r_texture_descriptor& desc);
  void destroy(r_texture_handle tex);
  void update(r_texture_handle tex, r_texture_sampler sampler);
  void update(r_texture_handle tex, r_texture_address addressing);
  void update_data(r_texture_handle tex,
                   const void* data, size_t size, size_t offset, r_texture_format format,
                   uint32 layer, uint32 mipmap);
  [[nodiscard]] r_texture_type query(r_texture_handle tex, r_query_type_t) const;
  [[nodiscard]] r_texture_format query(r_texture_handle tex, r_query_format_t) const;
  [[nodiscard]] r_texture_sampler query(r_texture_handle tex, r_query_sampler_t) const;
  [[nodiscard]] r_texture_address query(r_texture_handle tex, r_query_addressing_t) const;
  [[nodiscard]] uvec3 query(r_texture_handle tex, r_query_dim_t) const;
  [[nodiscard]] uint32 query(r_texture_handle tex, r_query_layer_t) const;
  [[nodiscard]] uint32 query(r_texture_handle tex, r_query_mipmaps_t) const;

  [[nodiscard]] r_framebuffer_handle create_framebuffer(const r_framebuffer_descriptor& desc);
  void destroy(r_framebuffer_handle fbo);
  void attach(r_framebuffer_handle fbo, r_texture_handle tex);
  [[nodiscard]] r_texture_handle dettach(r_framebuffer_handle fbo, uint32 index);

  [[nodiscard]] r_shader_handle create_shader(const r_shader_descriptor& desc);
  void destroy(r_shader_handle shader);
  [[nodiscard]] r_shader_type query(r_shader_handle shader, r_query_type_t) const;

  [[nodiscard]] r_pipeline_handle create_pipeline(const r_pipeline_descriptor& desc);
  void destroy(r_pipeline_handle pipeline);
  [[nodiscard]] r_shader_type query(r_pipeline_handle pipeline, r_query_stages_t) const;

public:
  template<typename T>
  requires(r_attrib_traits<T>::is_attrib)
  void push_uniform(r_uniform location, const T& data) {
    auto* ptr = _frame_arena.allocate<int32>(1);
    _uniforms.emplace_back(uniform_descriptor_t{
      .location = location,
      .type = r_attrib_traits<T>::tag,
      .data = std::construct_at(ptr, data),
    });
  }

public:
  bool valid() const { return bool{_ctx}; }
  explicit operator bool() const { return valid(); }

  r_api render_api() const { return _ctx_api; }
  r_window& win() { return *_win; }

private:
  weak_ref<r_window> _win;
  std::unique_ptr<r_platform_context> _ctx;
  r_api _ctx_api;

  res_container<buff_handle_data_t, r_buffer_handle> _buffers;
  res_container<tex_handle_data_t, r_texture_handle> _textures;
  res_container<fb_handle_data_t, r_framebuffer_handle> _framebuffers;
  res_container<shad_handle_data_t, r_shader_handle> _shaders;
  res_container<pip_handle_data_t, r_pipeline_handle> _pipelines;

  ntf::mem_arena _frame_arena;
  std::unordered_map<r_framebuffer_handle, std::vector<weak_ref<r_draw_cmd>>> _fb_cmds;
  std::vector<uniform_descriptor_t> _uniforms;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(r_context);
};

struct r_platform_context {
  virtual ~r_platform_context() = default;
  virtual r_api api_type() const = 0;

  virtual r_context::buff_handle create_buffer(const r_context::buffer_create_t& data) = 0;
  virtual void update_buffer_data(r_context::buff_handle buf,
                                  const r_context::buffer_update_t& data) = 0;
  virtual void destroy_buffer(r_context::buff_handle buf) = 0;

  virtual r_context::tex_handle create_texture(const r_context::tex_create_t& data) = 0;
  virtual void update_texture_param(r_context::tex_handle tex,
                                    const r_context::tex_param_t& data) = 0;
  virtual void update_texture_data(r_context::tex_handle tex,
                                   const r_context::tex_data_t& data) = 0;
  virtual void destroy_texture(r_context::tex_handle tex) = 0;

  virtual r_context::fb_handle create_framebuffer(const r_context::fbuff_create_t& data) = 0;
  virtual void destroy_framebuffer(r_context::fb_handle fb) = 0;

  virtual r_context::shad_handle create_shader(const r_context::shader_create_t& data) = 0;
  virtual void destroy_shader(r_context::fb_handle shader) = 0;

  virtual r_context::pip_handle create_pipeline(const r_context::pipeline_create_t& data) = 0;
  virtual void destroy_pipeline(r_context::pip_handle pipeline) = 0;

  virtual void submit(const r_context::command_queue& cmds) = 0;
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
//     return _ctx->query(_handle, r_query_dim);
//   }
//   [[nodiscard]] uvec2 dim2d() const {
//     auto d = dim(); return uvec2{d.x, d.y};
//   }
//
//   [[nodiscard]] uint32 layers() const {
//     return _ctx->query(_handle, r_query_layer);
//   }
//   [[nodiscard]] uint32 mipmaps() const {
//     return _ctx->query(_handle, r_query_mipmaps);
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

#undef SHOGLE_DECLARE_CONTEXT_HANDLE
