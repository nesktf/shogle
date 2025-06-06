#pragma once

#include "./context.hpp"

namespace ntf::meta {

template<typename T>
struct image_depth_traits {
  static constexpr bool is_specialized = false;
};

NTF_DECLARE_TAG_TYPE(image_depth_u8);
template<>
struct image_depth_traits<uint8> {
  static constexpr bool is_specialized = true;

  using tag_type = image_depth_u8_t;
  static constexpr tag_type tag = image_depth_u8;

  static constexpr bool is_signed = false;
  static constexpr bool is_floating = false;
  static constexpr std::string_view name = "8 bit unsigned";

  static constexpr optional<ntfr::image_format> parse_channels(uint8 flags) noexcept {
    const uint8 channels = flags & ntfr::IMAGE_DEPTH_CHANNELS_MASK;
    if (flags & ntfr::IMAGE_DEPTH_NORMALIZE_BIT) {
      switch (channels) {
        case 1u: return ntfr::image_format::r8nu;
        case 2u: return ntfr::image_format::rg8nu;
        case 3u: return ntfr::image_format::rgb8nu;
        case 4u: return ntfr::image_format::rgba8nu;
      }
    } else if (flags & ntfr::IMAGE_DEPTH_NONLINEAR_BIT) {
      switch (channels) {
        case 3u: return ntfr::image_format::srgb8u;
        case 4u: return ntfr::image_format::srgba8u;
      }
    } else {
      switch (channels) {
        case 1u: return ntfr::image_format::r8u;
        case 2u: return ntfr::image_format::rg8u;
        case 3u: return ntfr::image_format::rgb8u;
        case 4u: return ntfr::image_format::rgba8u;
      }
    }
    return nullopt;
  }
};

NTF_DECLARE_TAG_TYPE(image_depth_s8);
template<>
struct image_depth_traits<int8> {
  static constexpr bool is_specialized = true;

  using tag_type = image_depth_s8_t;
  static constexpr tag_type tag = image_depth_s8;

  static constexpr bool is_signed = true;
  static constexpr bool is_floating = false;
  static constexpr std::string_view name = "8 bit signed";

  static constexpr optional<ntfr::image_format> parse_channels(uint8 flags) noexcept {
    const uint8 channels = flags & ntfr::IMAGE_DEPTH_CHANNELS_MASK;
    if (flags & ntfr::IMAGE_DEPTH_NORMALIZE_BIT) {
      switch (channels) {
        case 1u: return ntfr::image_format::r8n;
        case 2u: return ntfr::image_format::rg8n;
        case 3u: return ntfr::image_format::rgb8n;
        case 4u: return ntfr::image_format::rgba8n;
      }
    } else {
      switch (channels) {
        case 1u: return ntfr::image_format::r8i;
        case 2u: return ntfr::image_format::rg8i;
        case 3u: return ntfr::image_format::rgb8i;
        case 4u: return ntfr::image_format::rgba8i;
      }
    }
    return nullopt;
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

  static constexpr optional<ntfr::image_format> parse_channels(uint8 flags) noexcept {
    const uint8 channels = flags & ntfr::IMAGE_DEPTH_CHANNELS_MASK;
    switch (channels) {
      case 1u: return ntfr::image_format::r16u;
      case 2u: return ntfr::image_format::rg16u;
      case 3u: return ntfr::image_format::rgb16u;
      case 4u: return ntfr::image_format::rgba16u;
    }
    return nullopt;
  }
};

NTF_DECLARE_TAG_TYPE(image_depth_s16);
template<>
struct image_depth_traits<int16> {
  static constexpr bool is_specialized = true;

  using tag_type = image_depth_s16_t;
  static constexpr tag_type tag = image_depth_s16;

  static constexpr bool is_signed = true;
  static constexpr bool is_floating = false;
  static constexpr std::string_view name = "16 bit signed";

  static constexpr optional<ntfr::image_format> parse_channels(uint8 flags) noexcept {
    const uint8 channels = flags & ntfr::IMAGE_DEPTH_CHANNELS_MASK;
    switch (channels) {
      case 1u: return ntfr::image_format::r16i;
      case 2u: return ntfr::image_format::rg16i;
      case 3u: return ntfr::image_format::rgb16i;
      case 4u: return ntfr::image_format::rgba16i;
    }
    return nullopt;
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

  static constexpr optional<ntfr::image_format> parse_channels(uint8 flags) noexcept {
    const uint8 channels = flags & ntfr::IMAGE_DEPTH_CHANNELS_MASK;
    switch (channels) {
      case 1u: return ntfr::image_format::r32f;
      case 2u: return ntfr::image_format::rg32f;
      case 3u: return ntfr::image_format::rgb32f;
      case 4u: return ntfr::image_format::rgba32f;
    }
    return nullopt;
  }
};

template<typename T>
concept image_depth_type = image_depth_traits<T>::is_specialized;

template<typename T>
struct image_dim_traits {
  static constexpr bool is_specialized = false;
};

template<>
struct image_dim_traits<extent1d> {
  static constexpr bool is_specialized = true;
  static constexpr bool allows_arrays = true;

  template<image_depth_type T>
  constexpr static size_t image_stride(extent1d extent) noexcept {
    return static_cast<size_t>(extent)*sizeof(T);
  }
  constexpr static extent3d extent_cast(extent1d extent) noexcept {
    return {static_cast<uint32>(extent), 1u, 1u};
  }
  constexpr static extent3d offset_cast(extent1d offset, uint32 layer = 0u) noexcept {
    return {static_cast<uint32>(offset), layer, 0u};
  }
};

template<>
struct image_dim_traits<extent2d> {
  static constexpr bool is_specialized = true;
  static constexpr bool allows_arrays = true;

  template<image_depth_type T>
  constexpr static size_t image_stride(extent2d extent) noexcept {
    return static_cast<size_t>(extent.x*extent.y)*sizeof(T);
  }
  constexpr static extent3d extent_cast(extent2d extent) noexcept {
    return {static_cast<uint32>(extent.x), static_cast<uint32>(extent.y), 1u};
  }
  constexpr static extent3d offset_cast(extent2d offset, uint32 layer = 0u) noexcept {
    return {static_cast<uint32>(offset.x), static_cast<uint32>(offset.y), layer};
  }
};

template<>
struct image_dim_traits<extent3d> {
  static constexpr bool is_specialized = true;
  static constexpr bool allows_arrays = false;

  template<image_depth_type T>
  constexpr static size_t image_stride(extent3d extent) noexcept {
    return static_cast<size_t>(extent.x*extent.y*extent.y)*sizeof(T);
  }
  constexpr static extent3d extent_cast(extent3d extent) noexcept {
    return extent;
  }
  constexpr static extent3d offset_cast(extent3d offset) noexcept {
    return offset;
  }
};

template<typename T>
concept image_dim_type = image_dim_traits<T>::is_specialized;

template<typename T>
concept image_array_dim_type = image_dim_type<T> && image_dim_traits<T>::allows_arrays;

} // namespace ntf::meta

namespace ntf::render {

template<meta::image_dim_type DimT>
extent3d image_extent_cast(const DimT& extent) noexcept {
  return meta::image_dim_traits<DimT>::extent_cast(extent);
}

template<meta::image_depth_type T, meta::image_dim_type DimT>
size_t image_stride(const DimT& extent) noexcept {
  return meta::image_dim_traits<DimT>::template image_stride<T>(extent);
}

template<meta::image_dim_type DimT>
extent3d image_offset_cast(const DimT& offset) noexcept {
  return meta::image_dim_traits<DimT>::offset_cast(offset);
}

template<meta::image_array_dim_type DimT>
extent3d image_offset_cast(const DimT& offset, uint32 layer) noexcept {
  return meta::image_dim_traits<DimT>::offset_cast(offset, layer);
}

enum class texture_type : uint8 {
  texture1d = 0,
  texture2d,
  texture3d,
  cubemap,
};

enum class texture_sampler : uint8 {
  nearest = 0,
  linear,
};

enum class texture_addressing : uint8 {
  repeat = 0,
  repeat_mirrored,
  clamp_edge,
  clamp_edge_mirrored,
  clamp_border,
};

enum class cubemap_face : uint8 {
  positive_x = 0,
  negative_x,
  positive_y,
  negative_y,
  positive_z,
  negative_z,
};

struct texture_data {
  cspan<image_data> images;
  bool generate_mipmaps;
};

struct typed_texture_desc {
  image_format format;
  texture_sampler sampler;
  texture_addressing addressing;
  extent3d extent;
  uint32 layers;
  uint32 levels;
  weak_cptr<texture_data> data;
};

struct texture_desc {
  texture_type type;
  image_format format;
  texture_sampler sampler;
  texture_addressing addressing;
  extent3d extent;
  uint32 layers;
  uint32 levels;
  weak_cptr<texture_data> data;
};

expect<texture_t> create_texture(context_t ctx, const texture_desc& desc);
void destroy_texture(texture_t tex) noexcept;

expect<void> texture_upload(texture_t tex, const texture_data& data);
expect<void> texture_set_sampler(texture_t tex, texture_sampler sampler);
expect<void> texture_set_addressing(texture_t tex, texture_addressing adressing);

texture_type texture_get_type(texture_t tex);
image_format texture_get_format(texture_t tex);
texture_sampler texture_get_sampler(texture_t tex);
texture_addressing texture_get_addressing(texture_t tex);
extent3d texture_get_extent(texture_t tex);
uint32 texture_get_layers(texture_t tex);
uint32 texture_get_levels(texture_t tex);
context_t texture_get_ctx(texture_t tex);
ctx_handle texture_get_id(texture_t tex);

constexpr inline uint32 to_cubemap_layer(cubemap_face face) {
  return static_cast<uint32>(face);
}

} // namespace ntf::render

namespace ntf::impl {

template<typename Derived>
class rtexture_ops {
  ntfr::texture_t _ptr() const noexcept(NTF_ASSERT_NOEXCEPT) {
    ntfr::texture_t ptr =  static_cast<const Derived&>(*this).get();
    NTF_ASSERT(ptr, "Invalid texture handle");
    return ptr;
  }

public:
  operator ntfr::texture_t() const noexcept(NTF_ASSERT_NOEXCEPT) { return _ptr(); }

  ntfr::expect<void> upload(const ntfr::texture_data& data) const {
    return ntfr::texture_upload(_ptr(), data);
  }
  ntfr::expect<void> sampler(ntfr::texture_sampler sampler) const {
    return ntfr::texture_set_sampler(_ptr(), sampler);
  }
  ntfr::expect<void> addressing(ntfr::texture_addressing addressing) const {
    return ntfr::texture_set_addressing(_ptr(), addressing);
  }

  ntfr::context_view context() const {
    return {ntfr::texture_get_ctx(_ptr())};
  }
  ntfr::texture_type type() const {
    return ntfr::texture_get_type(_ptr());
  }
  ntfr::image_format format() const {
    return ntfr::texture_get_format(_ptr());
  }
  ntfr::texture_sampler sampler() const {
    return ntfr::texture_get_sampler(_ptr());
  }
  ntfr::texture_addressing addressing() const {
    return ntfr::texture_get_addressing(_ptr());
  }
  ntfr::ctx_handle id() const {
    return ntfr::texture_get_id(_ptr());
  }
  extent3d extent() const {
    return ntfr::texture_get_extent(_ptr());
  }
  uint32 layers() const {
    return ntfr::texture_get_layers(_ptr());
  }
  uint32 levels() const {
    return ntfr::texture_get_levels(_ptr());
  }
  bool is_cubemap() const {
    return type() == ntfr::texture_type::cubemap;
  }
  bool is_array() const {
    return !is_cubemap() && layers() > 1;
  }
  bool has_mipmaps() const {
    return levels() > 0;
  }
};

template<typename Derived>
class rtexture_view : public rtexture_ops<Derived> {
protected:
  rtexture_view(ntfr::texture_t tex) noexcept :
    _tex{tex} {}

public:
  ntfr::texture_t get() const noexcept { return _tex; }

  bool empty() const noexcept { return _tex == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  ntfr::texture_t _tex;
};

template<typename Derived>
class rtexture_owning : public rtexture_ops<Derived> {
private:
  struct deleter_t {
    void operator()(ntfr::texture_t tex) noexcept {
      ntfr::destroy_texture(tex);
    }
  };
  using uptr_type = std::unique_ptr<std::remove_pointer_t<ntfr::texture_t>, deleter_t>;

protected:
  rtexture_owning(ntfr::texture_t tex) noexcept :
    _tex{tex} {}

public:
  ntfr::texture_t get() const noexcept { return _tex.get(); }
  [[nodiscard]] ntfr::texture_t release() noexcept { return _tex.release(); }

  bool empty() const noexcept { return _tex.get() == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  uptr_type _tex;
};

} // namespace ntf::impl

namespace ntf::render {

class texture_view : public ntf::impl::rtexture_view<texture_view> {
public:
  texture_view(texture_t tex) noexcept :
    ntf::impl::rtexture_view<texture_view>{tex} {}
};

class texture : public ntf::impl::rtexture_owning<texture> {
public:
  explicit texture(texture_t tex) noexcept :
    ntf::impl::rtexture_owning<texture>{tex} {}

public:
  static expect<texture> create(context_view ctx, const texture_desc& desc) {
    return ntfr::create_texture(ctx.get(), desc)
    .transform([](texture_t tex) -> texture {
      return texture{tex};
    });
  }

public:
  operator texture_view() const noexcept { return {this->get()}; }
};

template<texture_type tex_enum>
class typed_texture;

template<texture_type tex_enum>
class typed_texture_view : public ntf::impl::rtexture_view<typed_texture_view<tex_enum>> {
private:
  friend typed_texture<tex_enum>;

public:
  template<texture_type _tex_enum>
  friend typed_texture_view<_tex_enum> to_typed(texture_view tex) noexcept;

private:
  typed_texture_view(texture_t tex) noexcept :
    ntf::impl::rtexture_view<typed_texture_view<tex_enum>>{tex} {}

public:
  operator texture_view() const noexcept { return {this->get()}; }
};

template<texture_type tex_enum>
typed_texture_view<tex_enum> to_typed(texture_view tex) noexcept {
  texture_t ptr = nullptr;
  if (tex.type() == tex_enum) {
    ptr = tex.get();
  }
  return typed_texture_view<tex_enum>{ptr};
}

template<texture_type tex_enum>
class typed_texture : public ntf::impl::rtexture_owning<typed_texture<tex_enum>> {
public:
  template<texture_type _tex_enum>
  friend typed_texture<_tex_enum> to_typed(texture&& tex) noexcept;

private:
  typed_texture(texture_t tex) noexcept :
    ntf::impl::rtexture_owning<typed_texture<tex_enum>>{tex} {}

public:
  static expect<typed_texture> create(context_view ctx, const typed_texture_desc& desc) {
    return ntfr::create_texture(ctx.get(), {
      .type = tex_enum,
      .format = desc.format,
      .sampler = desc.sampler,
      .addressing = desc.addressing,
      .extent = desc.extent,
      .layers = desc.layers,
      .levels = desc.levels,
      .data = desc.data,
    })
    .transform([](texture_t tex) -> typed_texture {
      return typed_texture{tex};
    });
  }

public:
  operator texture_view() const noexcept { return {this->get()}; }
  operator typed_texture_view<tex_enum>() const noexcept { return {this->get()}; }
};

template<texture_type tex_enum>
typed_texture<tex_enum> to_typed(texture&& tex) noexcept {
  texture_t ptr = nullptr;
  if (tex.type() == tex_enum) {
    ptr = tex.release();
  }
  return typed_texture<tex_enum>{ptr};
}

using texture1d = typed_texture<texture_type::texture1d>;
using texture1d_view = typed_texture_view<texture_type::texture1d>;

using texture2d = typed_texture<texture_type::texture2d>;
using texture2d_view = typed_texture_view<texture_type::texture2d>;

using texture3d = typed_texture<texture_type::texture3d>;
using texture3d_view = typed_texture_view<texture_type::texture3d>;

using cubemap_texture = typed_texture<texture_type::cubemap>;
using cubemap_texture_view = typed_texture_view<texture_type::cubemap>;

} // namespace ntf::render
