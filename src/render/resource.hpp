#pragma once

#include "./context.hpp"

namespace ntf {

namespace impl {

template<typename T>
struct r_buffer_gets {
  [[nodiscard]] r_buffer_type type() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx.buffer_type(self._handle);
  }
  [[nodiscard]] size_t size() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx.buffer_size(self._handle);
  }
};

template<typename T>
struct r_buffer_sets {
  static r_expected<T> create(r_context_view ctx, const r_buffer_descriptor& desc) noexcept {
    auto buf = ctx.buffer_create(desc);
    if (!buf) {
      return unexpected{std::move(buf.error())};
    }
    return T{ctx, *buf};
  }
  static T create(unchecked_t, r_context_view ctx, const r_buffer_descriptor& desc) {
    auto buf = ctx.buffer_create(::ntf::unchecked, desc);
    NTF_ASSERT(buf);
    return T{ctx, buf};
  }
  template<typename U, size_t N>
  static r_expected<T> create(r_context_view ctx, U (&arr)[N],
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
  static T create(unchecked_t, r_context_view ctx, U (&arr)[N],
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
    return self._ctx.buffer_update(self._handle, data);
  }
  void update(unchecked_t, const r_buffer_data& data) {
    T& self = static_cast<T&>(*this);
    self._ctx.buffer_update(::ntf::unchecked, self._handle, data);
  }
  r_expected<void*> map(size_t offset, size_t len) {
    T& self = static_cast<T&>(*this);
    return self._ctx.buffer_map(self._handle, offset, len);
  }
  void* map(unchecked_t, size_t offset, size_t len) {
    T& self = static_cast<T&>(*this);
    return self._ctx.buffer_map(::ntf::unchecked, self._handle, offset, len);
  }
  void unmap(void* ptr) {
    T& self = static_cast<T&>(*this);
    self._ctx.buffer_unmap(self._handle, ptr);
  }
};

template<typename T>
struct r_texture_gets {
  [[nodiscard]] r_texture_type type() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx.texture_type(self._handle);
  }
  [[nodiscard]] r_texture_format format() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx.texture_format(self._handle);
  }
  [[nodiscard]] r_texture_sampler sampler() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx.texture_sampler(self._handle);
  }
  [[nodiscard]] r_texture_address addressing() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx.texture_addressing(self._handle);
  }
  [[nodiscard]] uvec3 extent() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx.texture_extent(self._handle);
  }
  [[nodiscard]] uint32 layers() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx.texture_layers(self._handle);
  }
  [[nodiscard]] uint32 levels() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx.texture_levels(self._handle);
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
  static r_expected<T> create(r_context_view ctx, const r_texture_descriptor& desc) noexcept {
    auto tex = ctx.texture_create(desc);
    if (!tex) {
      return unexpected{std::move(tex.error())};
    }
    return T{ctx, *tex};
  }
  static T create(unchecked_t, r_context_view ctx, const r_texture_descriptor& desc) {
    auto tex = ctx.texture_create(::ntf::unchecked, desc);
    NTF_ASSERT(tex);
    return T{ctx, tex};
  }

  r_expected<void> update(const r_texture_data& data) noexcept {
    T& self = static_cast<T&>(*this);
    return self._ctx.texture_update(self._handle, data);
  }
  void update(unchecked_t, const r_texture_data& data) {
    T& self = static_cast<T&>(*this);
    self._ctx.texture_update(::ntf::unchecked, self._handle, data);
  }
  r_expected<void> update(span_view<r_image_data> images, bool gen_mipmaps) noexcept {
    T& self = static_cast<T&>(*this);
    return self._ctx.texture_update(self._handle, images, gen_mipmaps);
  }
  void update(unchecked_t, span_view<r_image_data> images, bool gen_mipmaps) {
    T& self = static_cast<T&>(*this);
    self._ctx.texture_update(::ntf::unchecked, self._handle, images, gen_mipmaps);
  }
  r_expected<void> sampler(r_texture_sampler sampler) noexcept {
    T& self = static_cast<T&>(*this);
    return self._ctx.texture_sampler(self._handle, sampler);
  }
  void sampler(unchecked_t, r_texture_sampler sampler) {
    T& self = static_cast<T&>(*this);
    self._ctx.texture_sampler(::ntf::unchecked, self._handle, sampler);
  }
  r_expected<void> addressing(r_texture_address addressing) noexcept {
    T& self = static_cast<T&>(*this);
    return self._ctx.texture_addressing(self._handle, addressing);
  }
  void sampler(unchecked_t, r_texture_address addressing) {
    T& self = static_cast<T&>(*this);
    self._ctx.texture_addressing(::ntf::unchecked, self._handle, addressing);
  }
};

template<typename T>
struct r_framebuffer_gets {
  [[nodiscard]] r_clear_flag clear_flags() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx.framebuffer_clear(self._handle);
  }
  [[nodiscard]] uvec4 viewport() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx.framebuffer_viewport(self._handle);
  }
  [[nodiscard]] color4 clear_color() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx.framebuffer_color(self._handle);
  }
};

template<typename T>
struct r_framebuffer_sets {
  static r_expected<T> create(r_context_view ctx, const r_framebuffer_descriptor& desc) noexcept {
    auto fbo = ctx.framebuffer_create(desc);
    if (!fbo) {
      return unexpected{std::move(fbo.error())};
    }
    return T{ctx, *fbo};
  }
  static T create(unchecked_t, r_context_view ctx, const r_framebuffer_descriptor& desc) {
    auto fbo = ctx.framebuffer_create(::ntf::unchecked, desc);
    NTF_ASSERT(fbo);
    return T{ctx, fbo};
  }

  void clear_flags(r_clear_flag flags) { 
    T& self = static_cast<T&>(*this);
    self._ctx.framebuffer_clear(self._handle, flags);
  }
  void clear_color(const color4& color) {
    T& self = static_cast<T&>(*this);
    self._ctx.framebuffer_color(self._handle, color);
  }
  void viewport(const uvec4& vp) { 
    T& self = static_cast<T&>(*this);
    self._ctx.framebuffer_viewport(self._handle, vp);
  }
};

template<typename T>
struct r_shader_gets {
  [[nodiscard]] r_shader_type type() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx.shader_type(self._handle);
  }
};

template<typename T>
struct r_shader_sets {
  static r_expected<T> create(r_context_view ctx, const r_shader_descriptor& desc) noexcept {
    auto shad = ctx.shader_create(desc);
    if (!shad) {
      return unexpected{std::move(shad.error())};
    }
    return T{ctx, *shad};
  }
  static T create(unchecked_t, r_context_view ctx, const r_shader_descriptor& desc) {
    auto shad = ctx.shader_create(::ntf::unchecked, desc);
    NTF_ASSERT(shad);
    return T{ctx, shad};
  }
};

template<typename T>
struct r_pipeline_gets {
  [[nodiscard]] r_stages_flag stages() const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx.pipeline_stages(self._handle);
  }
  [[nodiscard]] optional<r_uniform> uniform(std::string_view name) const noexcept {
    const T& self = static_cast<const T&>(*this);
    return self._ctx.pipeline_uniform(self._handle, name);
  }
  [[nodiscard]] r_uniform uniform(unchecked_t, std::string_view name) const {
    const T& self = static_cast<const T&>(*this);
    return self._ctx.pipeline_uniform(::ntf::unchecked, self._handle, name);
  }
};

template<typename T>
struct r_pipeline_sets {
  static r_expected<T> create(r_context_view ctx, const r_pipeline_descriptor& desc) noexcept {
    auto pip = ctx.pipeline_create(desc);
    if (!pip) {
      return unexpected{std::move(pip.error())};
    }
    return T{ctx, *pip};
  }
  static T create(unchecked_t, r_context_view ctx, const r_pipeline_descriptor& desc) {
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
  r_owning_type(r_context_view ctx, Handle handle) noexcept :
    _ctx{ctx}, _handle{handle} {}

public:
  void reset() noexcept {
    if (_handle.valid()) {
      _ctx.destroy(_handle);
    }
    _handle = Handle{};
  }
  [[nodiscard]] Handle release() {
    NTF_ASSERT(_handle.valid());
    return _handle;
  }

public:
  [[nodiscard]] Handle handle() const { return _handle; }
  [[nodiscard]] r_context_view context() const { return _ctx; }
  explicit operator Handle() const { return _handle; }
  operator view_type() const { return view_type{_ctx, _handle}; }

private:
  r_context_view _ctx;
  Handle _handle;

public:
  ~r_owning_type() noexcept { reset(); }
  r_owning_type(const r_owning_type&) = delete;
  r_owning_type& operator=(const r_owning_type&) = delete;
  r_owning_type(r_owning_type&& other) noexcept :
    _ctx{std::move(other._ctx)}, _handle{std::move(other._handle)} { other._handle = Handle{}; }
  r_owning_type& operator=(r_owning_type&& other) noexcept {
    if (std::addressof(other) == this) {
      return *this;
    }
    reset();
    _ctx = std::move(other._ctx);
    _handle = std::move(other._handle);
    other._handle = Handle{};
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

} // namespace ntf
