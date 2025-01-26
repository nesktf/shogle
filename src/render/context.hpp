#pragma once

#include "./render.hpp"
#include "./window.hpp"

#include "../stl/expected.hpp"
#include "../stl/optional.hpp"
#include "../stl/arena.hpp"

namespace ntf {

class r_draw_cmd;

struct r_platform_context;

struct r_context_params {
  optional<r_api> use_api;
};

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

private:
  r_context(r_error err) noexcept;
  r_context(r_window& win, std::unique_ptr<r_platform_context> ctx, command_map map) noexcept;

public:
  static r_context create(r_window& win, const r_context_params& params = {}) noexcept;

public:
  void start_frame() noexcept;
  void end_frame() noexcept;
  void device_wait() noexcept;

public:
  [[nodiscard]] r_expected<r_buffer_handle> buffer_create(
                                                         const r_buffer_descriptor& desc) noexcept;
  [[nodiscard]] r_buffer_handle buffer_create(unchecked_t,
                                                         const r_buffer_descriptor& desc);
  void destroy(r_buffer_handle buff) noexcept;

  r_expected<void> buffer_update(r_buffer_handle buff, const r_buffer_data& desc) noexcept;
  void buffer_update(unchecked_t, r_buffer_handle buff, const r_buffer_data& desc);

  [[nodiscard]] r_buffer_type buffer_type(r_buffer_handle buff) const;
  [[nodiscard]] size_t buffer_size(r_buffer_handle buff) const;

public:
  [[nodiscard]] r_expected<r_texture_handle> texture_create(
                                                        const r_texture_descriptor& desc) noexcept;
  [[nodiscard]] r_texture_handle texture_create(unchecked_t,
                                                        const r_texture_descriptor& desc);
  void destroy(r_texture_handle tex) noexcept;

  r_expected<void> texture_update(r_texture_handle tex, const r_texture_data& data) noexcept;
  void texture_update(unchecked_t, r_texture_handle tex, const r_texture_data& data);
  r_expected<void> texture_update(r_texture_handle tex,
                                  span_view<r_image_data> images, bool gen_mipmaps) noexcept;
  void texture_update(unchecked_t, r_texture_handle tex,
                      span_view<r_image_data> images, bool gen_mipmaps);
  r_expected<void> texture_sampler(r_texture_handle tex, r_texture_sampler sampler) noexcept;
  void texture_sampler(unchecked_t, r_texture_handle tex, r_texture_sampler sampler);
  r_expected<void> texture_addressing(r_texture_handle tex, r_texture_address addressing) noexcept;
  void texture_addressing(unchecked_t, r_texture_handle tex, r_texture_address addressing);

  [[nodiscard]] r_texture_type texture_type(r_texture_handle tex) const;
  [[nodiscard]] r_texture_format texture_format(r_texture_handle tex) const;
  [[nodiscard]] r_texture_sampler texture_sampler(r_texture_handle tex) const;
  [[nodiscard]] r_texture_address texture_addressing(r_texture_handle tex) const;
  [[nodiscard]] uvec3 texture_extent(r_texture_handle tex) const;
  [[nodiscard]] uint32 texture_layers(r_texture_handle tex) const;
  [[nodiscard]] uint32 texture_levels(r_texture_handle tex) const;

public:
  [[nodiscard]] r_expected<r_framebuffer_handle> framebuffer_create(
                                                    const r_framebuffer_descriptor& desc) noexcept;
  [[nodiscard]] r_framebuffer_handle framebuffer_create(unchecked_t,
                                                    const r_framebuffer_descriptor& desc);
  void destroy(r_framebuffer_handle fbo) noexcept;

  void framebuffer_clear(r_framebuffer_handle fbo, r_clear_flag flags);
  void framebuffer_viewport(r_framebuffer_handle fbo, uvec4 vp);
  void framebuffer_color(r_framebuffer_handle fbo, color4 color);

  [[nodiscard]] r_clear_flag framebuffer_clear(r_framebuffer_handle fbo) const;
  [[nodiscard]] uvec4 framebuffer_viewport(r_framebuffer_handle fbo) const;
  [[nodiscard]] color4 framebuffer_color(r_framebuffer_handle fbo) const;

public:
  [[nodiscard]] r_expected<r_shader_handle> shader_create(
                                                        const r_shader_descriptor& desc) noexcept;
  [[nodiscard]] r_shader_handle shader_create(unchecked_t,
                                                        const r_shader_descriptor& desc);
  void destroy(r_shader_handle shader) noexcept;

  [[nodiscard]] r_shader_type shader_type(r_shader_handle shader) const;

public:
  [[nodiscard]] r_expected<r_pipeline_handle> pipeline_create(
                                                       const r_pipeline_descriptor& desc) noexcept;
  [[nodiscard]] r_pipeline_handle pipeline_create(unchecked_t,
                                                       const r_pipeline_descriptor& desc);
  void destroy(r_pipeline_handle pipeline) noexcept;

  [[nodiscard]] r_stages_flag pipeline_stages(r_pipeline_handle pipeline) const;
  [[nodiscard]] optional<r_uniform> pipeline_uniform(r_pipeline_handle pipeline,
                                                     std::string_view name) const noexcept;
  [[nodiscard]] r_uniform pipeline_uniform(unchecked_t, r_pipeline_handle pipeline,
                                           std::string_view name) const;

public:
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
  bool valid() const { return !_err.has_value(); }
  explicit operator bool() const { return valid(); }

  r_error& error() { return _err.value(); }
  const r_error& error() const { return _err.value(); }

  r_api render_api() const { return _ctx_meta.api; }
  std::string_view name_str() const;
  r_window& win() { return *_win; }

private:
  void _init_buffer(buff_store_t& buff, const r_buffer_descriptor& desc);
  void _init_texture(tex_store_t& tex, const r_texture_descriptor& desc);
  void _init_framebuffer(r_framebuffer_handle handle, fb_store_t& fbo,
                         const r_framebuffer_descriptor& desc);
  void _init_shader(shader_store_t& shad, const r_shader_descriptor& desc);
  std::unique_ptr<vertex_attrib_t> _copy_pipeline_layout(const r_pipeline_descriptor& desc);
  void _init_pipeline(pipeline_store_t& pip, const r_pipeline_descriptor& desc,
                      std::unique_ptr<vertex_attrib_t> layout, uniform_map uniforms);

private:
  optional<r_error> _err;
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
  virtual void destroy_buffer(r_buffer_handle buf) noexcept = 0;

  virtual r_texture_handle create_texture(const r_texture_descriptor& desc) = 0;
  virtual void update_texture(r_texture_handle tex, const r_texture_data& desc) = 0;
  virtual void destroy_texture(r_texture_handle tex) noexcept = 0;

  virtual r_shader_handle create_shader(const r_shader_descriptor& desc) = 0;
  virtual void destroy_shader(r_shader_handle shader) noexcept = 0;

  virtual r_pipeline_handle create_pipeline(const r_pipeline_descriptor& desc,
                                            weak_ref<r_context::vertex_attrib_t> attrib,
                                            r_context::uniform_map& uniforms) = 0;
  virtual void destroy_pipeline(r_pipeline_handle pipeline) noexcept = 0;

  virtual r_framebuffer_handle create_framebuffer(const r_framebuffer_descriptor& desc) = 0;
  virtual void destroy_framebuffer(r_framebuffer_handle fb) noexcept = 0;

  virtual void submit(const r_context::command_map&) = 0;

  virtual void device_wait() noexcept {}
};

namespace impl {

template<typename T>
struct r_buffer_gets {
  [[nodiscard]] r_buffer_type type() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx->buffer_type(self._handle);
  }
  [[nodiscard]] size_t size() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx->buffer_size(self._handle);
  }
};

template<typename T>
struct r_buffer_sets {
  static r_expected<T> create(r_context& ctx, const r_buffer_descriptor& desc) noexcept {
    auto buf = ctx.buffer_create(desc);
    if (!buf) {
      return unexpected{std::move(buf.error())};
    }
    return T{ctx, *buf};
  }
  static T create(unchecked_t, r_context& ctx, const r_buffer_descriptor& desc) {
    auto buf = ctx.buffer_create(::ntf::unchecked, desc);
    NTF_ASSERT(buf);
    return T{ctx, buf};
  }
  template<typename U, size_t N>
  static r_expected<T> create(r_context& ctx, U (&arr)[N],
                              r_buffer_type type, r_buffer_flag flags) {
    r_buffer_data bdata{
      .data = std::addressof(arr),
      .size = sizeof(arr),
      .offset = 0,
    };
    return create(ctx, {
      .type = type,
      .flags = flags,
      .size = sizeof(arr),
      .data = &bdata,
    });
  }
  template<typename U, size_t N>
  static T create(unchecked_t, r_context& ctx, U (&arr)[N],
                  r_buffer_type type, r_buffer_flag flags) {
    r_buffer_data bdata{
      .data = std::addressof(arr),
      .size = sizeof(arr),
      .offset = 0,
    };
    return create(::ntf::unchecked, ctx, {
      .type = type,
      .flags = flags,
      .size = sizeof(arr),
      .data = &bdata
    });
  }

  r_expected<void> update(const r_buffer_data& data) {
    T& self = static_cast<T&>(*this);
    return self._ctx->buffer_update(self._handle, data);
  }
  void update(unchecked_t, const r_buffer_data& data) {
    T& self = static_cast<T&>(*this);
    self._ctx->buffer_update(::ntf::unchecked, self._handle, data);
  }
};

template<typename T>
struct r_texture_gets {
  [[nodiscard]] r_texture_type type() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx->texture_type(self._handle);
  }
  [[nodiscard]] r_texture_format format() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx->texture_format(self._handle);
  }
  [[nodiscard]] r_texture_sampler sampler() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx->texture_sampler(self._handle);
  }
  [[nodiscard]] r_texture_address addressing() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx->texture_addressing(self._handle);
  }
  [[nodiscard]] uvec3 extent() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx->texture_extent(self._handle);
  }
  [[nodiscard]] uint32 layers() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx->texture_layers(self._handle);
  }
  [[nodiscard]] uint32 levels() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx->texture_levels(self._handle);
  }
  [[nodiscard]] bool is_cubemap() const {
    return type() == r_texture_type::cubemap;
  }
  [[nodiscard]] uvec2 extent2d() const {
    auto d = extent();
    return uvec2{d.x, d.y};
  }
  [[nodiscard]] bool is_array() const {
    return !is_cubemap() && layers() > 1;
  }
  [[nodiscard]] bool has_mipmaps() const {
    return levels() > 0;
  }
};

template<typename T>
struct r_texture_sets {
  static r_expected<T> create(r_context& ctx, const r_texture_descriptor& desc) noexcept {
    auto tex = ctx.texture_create(desc);
    if (!tex) {
      return unexpected{std::move(tex.error())};
    }
    return T{ctx, *tex};
  }
  static T create(unchecked_t, r_context& ctx, const r_texture_descriptor& desc) {
    auto tex = ctx.texture_create(::ntf::unchecked, desc);
    NTF_ASSERT(tex);
    return T{ctx, tex};
  }

  r_expected<void> update(const r_texture_data& data) noexcept {
    T& self = static_cast<T&>(*this);
    return self._ctx->texture_update(self._handle, data);
  }
  void update(unchecked_t, const r_texture_data& data) {
    T& self = static_cast<T&>(*this);
    self._ctx->texture_update(::ntf::unchecked, self._handle, data);
  }
  r_expected<void> update(span_view<r_image_data> images, bool gen_mipmaps) noexcept {
    T& self = static_cast<T&>(*this);
    return self._ctx->texture_update(self._handle, images, gen_mipmaps);
  }
  void update(unchecked_t, span_view<r_image_data> images, bool gen_mipmaps) {
    T& self = static_cast<T&>(*this);
    self._ctx->texture_update(::ntf::unchecked, self._handle, images, gen_mipmaps);
  }
  r_expected<void> sampler(r_texture_sampler sampler) noexcept {
    T& self = static_cast<T&>(*this);
    return self._ctx->texture_sampler(self._handle, sampler);
  }
  void sampler(unchecked_t, r_texture_sampler sampler) {
    T& self = static_cast<T&>(*this);
    self._ctx->texture_sampler(::ntf::unchecked, self._handle, sampler);
  }
  r_expected<void> addressing(r_texture_address addressing) noexcept {
    T& self = static_cast<T&>(*this);
    return self._ctx->texture_addressing(self._handle, addressing);
  }
  void sampler(unchecked_t, r_texture_address addressing) {
    T& self = static_cast<T&>(*this);
    self._ctx->texture_addressing(::ntf::unchecked, self._handle, addressing);
  }
};

template<typename T>
struct r_framebuffer_gets {
  [[nodiscard]] r_clear_flag clear_flags() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx->framebuffer_clear(self._handle);
  }
  [[nodiscard]] uvec4 viewport() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx->framebuffer_viewport(self._handle);
  }
  [[nodiscard]] color4 clear_color() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx->framebuffer_color(self._handle);
  }
};

template<typename T>
struct r_framebuffer_sets {
  static r_expected<T> create(r_context& ctx, const r_framebuffer_descriptor& desc) noexcept {
    auto fbo = ctx.framebuffer_create(desc);
    if (!fbo) {
      return unexpected{std::move(fbo.error())};
    }
    return T{ctx, *fbo};
  }
  static T create(unchecked_t, r_context& ctx, const r_framebuffer_descriptor& desc) {
    auto fbo = ctx.framebuffer_create(::ntf::unchecked, desc);
    NTF_ASSERT(fbo);
    return T{ctx, fbo};
  }

  void clear_flags(r_clear_flag flags) { 
    T& self = static_cast<T&>(*this);
    self._ctx->framebuffer_clear(self._handle, flags);
  }
  void clear_color(const color4& color) {
    T& self = static_cast<T&>(*this);
    self._ctx->framebuffer_color(self._handle, color);
  }
  void viewport(const uvec4& vp) { 
    T& self = static_cast<T&>(*this);
    self._ctx->framebuffer_viewport(self._handle, vp);
  }
};

template<typename T>
struct r_shader_gets {
  [[nodiscard]] r_shader_type type() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx->shader_type(self._handle);
  }
};

template<typename T>
struct r_shader_sets {
  static r_expected<T> create(r_context& ctx, const r_shader_descriptor& desc) noexcept {
    auto shad = ctx.shader_create(desc);
    if (!shad) {
      return unexpected{std::move(shad.error())};
    }
    return T{ctx, *shad};
  }
  static T create(unchecked_t, r_context& ctx, const r_shader_descriptor& desc) {
    auto shad = ctx.shader_create(::ntf::unchecked, desc);
    NTF_ASSERT(shad);
    return T{ctx, shad};
  }
};

template<typename T>
struct r_pipeline_gets {
  [[nodiscard]] r_stages_flag stages() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx->pipeline_stages(self._handle);
  }
  [[nodiscard]] optional<r_uniform> uniform(std::string_view name) const noexcept {
    const T& self = static_cast<const T&>(*this);
    return self._ctx->pipeline_uniform(self._handle, name);
  }
  [[nodiscard]] r_uniform uniform(unchecked_t, std::string_view name) const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx->pipeline_uniform(::ntf::unchecked, self._handle, name);
  }
};

template<typename T>
struct r_pipeline_sets {
  static r_expected<T> create(r_context& ctx, const r_pipeline_descriptor& desc) noexcept {
    auto pip = ctx.pipeline_create(desc);
    if (!pip) {
      return unexpected{std::move(pip.error())};
    }
    return T{ctx, *pip};
  }
  static T create(unchecked_t, r_context& ctx, const r_pipeline_descriptor& desc) {
    auto pip = ctx.pipeline_create(::ntf::unchecked, desc);
    NTF_ASSERT(pip);
    return T{ctx, pip};
  }
};

template<
  typename Handle,
  template<typename> class Gets,
  template<typename> class Sets>
class r_owning_type :
  public Gets<r_owning_type<Handle, Gets, Sets>>,
  public Sets<r_owning_type<Handle, Gets, Sets>> {
public:
  class view_type : public Gets<view_type> {
  public:
    view_type() = default;
    view_type(weak_ref<r_context> ctx, Handle handle) noexcept :
      _ctx{ctx}, _handle{handle} {}

  public:
    [[nodiscard]] Handle handle() const { return _handle; }
    [[nodiscard]] weak_ref<r_context> context() const { return _ctx; }
    explicit operator Handle() const { return _handle; }

  private:
    weak_ref<r_context> _ctx;
    Handle _handle;

  private:
    friend Gets<view_type>;
  };

public:
  r_owning_type() = default;
  r_owning_type(r_context& ctx, Handle handle) noexcept :
    _ctx{ctx}, _handle{handle} {}

public:
  void reset() noexcept {
    if (_ctx.valid()) {
      _ctx->destroy(_handle);
      _ctx.reset();
    }
  }
  [[nodiscard]] Handle release() {
    NTF_ASSERT(_ctx.valid());
    _ctx.reset();
    return _handle;
  }

public:
  [[nodiscard]] Handle handle() const { return _handle; }
  [[nodiscard]] weak_ref<r_context> context() const { return _ctx; }
  explicit operator Handle() const { return _handle; }
  operator view_type() const { return view_type{_ctx, _handle}; }

private:
  weak_ref<r_context> _ctx;
  Handle _handle;

public:
  ~r_owning_type() noexcept { reset(); }
  r_owning_type(const r_owning_type&) = delete;
  r_owning_type& operator=(const r_owning_type&) = delete;
  r_owning_type(r_owning_type&& other) noexcept :
    _ctx{std::move(other._ctx)}, _handle{std::move(other._handle)} { other._ctx.reset(); }
  r_owning_type& operator=(r_owning_type&& other) noexcept {
    if (std::addressof(other) == this) {
      return *this;
    }
    reset();
    _ctx = std::move(other._ctx);
    _handle = std::move(other._handle);
    other._ctx.reset();
    return *this;
  }

private:
  friend Gets<r_owning_type<Handle, Gets, Sets>>;
  friend Sets<r_owning_type<Handle, Gets, Sets>>;
};

} // namespace impl

using r_buffer = impl::r_owning_type<
  r_buffer_handle, impl::r_buffer_gets, impl::r_buffer_sets>;
using r_buffer_view = r_buffer::view_type;

using r_texture = impl::r_owning_type<
  r_texture_handle, impl::r_texture_gets, impl::r_texture_sets>;
using r_texture_view = r_texture::view_type;

using r_framebuffer = impl::r_owning_type<
  r_framebuffer_handle, impl::r_framebuffer_gets, impl::r_framebuffer_sets>;
using r_framebuffer_view = r_framebuffer::view_type;

using r_shader = impl::r_owning_type<
  r_shader_handle, impl::r_shader_gets, impl::r_shader_sets>;
using r_shader_view = r_shader::view_type;

using r_pipeline = impl::r_owning_type<
  r_pipeline_handle, impl::r_pipeline_gets, impl::r_pipeline_sets>;
using r_pipeline_view = r_pipeline::view_type;

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
