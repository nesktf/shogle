#pragma once

#include <shogle/render/context.hpp>

namespace ntf::render {

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
  ntfr::extent3d extent() const {
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
  texture_view() noexcept :
    ntf::impl::rtexture_view<texture_view>{nullptr} {}

  texture_view(texture_t tex) noexcept :
    ntf::impl::rtexture_view<texture_view>{tex} {}
};

class texture : public ntf::impl::rtexture_owning<texture> {
public:
  texture() noexcept :
    ntf::impl::rtexture_owning<texture>{nullptr} {}

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

public:
  typed_texture_view() noexcept :
    ntf::impl::rtexture_view<typed_texture_view<tex_enum>>{nullptr} {}

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

public:
  typed_texture() noexcept :
    ntf::impl::rtexture_owning<typed_texture<tex_enum>>{nullptr} {}

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
