#pragma once

#include "./renderer.hpp"

namespace ntf {

constexpr inline uint32 r_cubemap_layer(r_cubemap_face face) {
  return static_cast<uint32>(face);
}

template<typename T>
concept tex_depth_type = same_as_any<T,
  uint8, int8,
  uint16, int16,
  float32
>;

constexpr uint8 TEX_DEPTH_CHANNELS_MASK = 0b00000111;
constexpr uint8 TEX_DEPTH_NORMALIZE_BIT = 0b10000000;
constexpr uint8 TEX_DEPTH_NONLINEAR_BIT = 0b01000000;

template<tex_depth_type T>
struct tex_depth_traits;

NTF_DECLARE_TAG_TYPE(tex_depth_u8);
template<>
struct tex_depth_traits<uint8> {
  using tag_type = tex_depth_u8_t;
  static constexpr tag_type tag = tex_depth_u8;

  static constexpr bool is_signed = false;
  static constexpr bool is_floating = false;
  static constexpr std::string_view name = "8 bit unsigned";

  static constexpr optional<r_texture_format> parse_channels(uint8 flags) noexcept {
    const uint8 channels = flags & TEX_DEPTH_CHANNELS_MASK;
    if (flags & TEX_DEPTH_NORMALIZE_BIT) {
      switch (channels) {
        case 1u: return r_texture_format::r8nu;
        case 2u: return r_texture_format::rg8nu;
        case 3u: return r_texture_format::rgb8nu;
        case 4u: return r_texture_format::rgba8nu;
      }
    } else if (flags & TEX_DEPTH_NONLINEAR_BIT) {
      switch (channels) {
        case 3u: return r_texture_format::srgb8u;
        case 4u: return r_texture_format::srgba8u;
      }
    } else {
      switch (channels) {
        case 1u: return r_texture_format::r8u;
        case 2u: return r_texture_format::rg8u;
        case 3u: return r_texture_format::rgb8u;
        case 4u: return r_texture_format::rgba8u;
      }
    }
    return nullopt;
  }
};

NTF_DECLARE_TAG_TYPE(tex_depth_s8);
template<>
struct tex_depth_traits<int8> {
  using tag_type = tex_depth_s8_t;
  static constexpr tag_type tag = tex_depth_s8;

  static constexpr bool is_signed = true;
  static constexpr bool is_floating = false;
  static constexpr std::string_view name = "8 bit signed";

  static constexpr optional<r_texture_format> parse_channels(uint8 flags) noexcept {
    const uint8 channels = flags & TEX_DEPTH_CHANNELS_MASK;
    if (flags & TEX_DEPTH_NORMALIZE_BIT) {
      switch (channels) {
        case 1u: return r_texture_format::r8n;
        case 2u: return r_texture_format::rg8n;
        case 3u: return r_texture_format::rgb8n;
        case 4u: return r_texture_format::rgba8n;
      }
    } else {
      switch (channels) {
        case 1u: return r_texture_format::r8i;
        case 2u: return r_texture_format::rg8i;
        case 3u: return r_texture_format::rgb8i;
        case 4u: return r_texture_format::rgba8i;
      }
    }
    return nullopt;
  }
};

NTF_DECLARE_TAG_TYPE(tex_depth_u16);
template<>
struct tex_depth_traits<uint16> {
  using tag_type = tex_depth_u16_t;
  static constexpr tag_type tag = tex_depth_u16;

  static constexpr bool is_signed = false;
  static constexpr bool is_floating = false;
  static constexpr std::string_view name = "16 bit unsigned";

  static constexpr optional<r_texture_format> parse_channels(uint8 flags) noexcept {
    const uint8 channels = flags & TEX_DEPTH_CHANNELS_MASK;
    switch (channels) {
      case 1u: return r_texture_format::r16u;
      case 2u: return r_texture_format::rg16u;
      case 3u: return r_texture_format::rgb16u;
      case 4u: return r_texture_format::rgba16u;
    }
    return nullopt;
  }
};

NTF_DECLARE_TAG_TYPE(tex_depth_s16);
template<>
struct tex_depth_traits<int16> {
  using tag_type = tex_depth_s16_t;
  static constexpr tag_type tag = tex_depth_s16;

  static constexpr bool is_signed = true;
  static constexpr bool is_floating = false;
  static constexpr std::string_view name = "16 bit signed";

  static constexpr optional<r_texture_format> parse_channels(uint8 flags) noexcept {
    const uint8 channels = flags & TEX_DEPTH_CHANNELS_MASK;
    switch (channels) {
      case 1u: return r_texture_format::r16i;
      case 2u: return r_texture_format::rg16i;
      case 3u: return r_texture_format::rgb16i;
      case 4u: return r_texture_format::rgba16i;
    }
    return nullopt;
  }
};

NTF_DECLARE_TAG_TYPE(tex_depth_f32);
template<>
struct tex_depth_traits<float32> {
  using tag_type = tex_depth_f32_t;
  static constexpr tag_type tag = tex_depth_f32;

  static constexpr bool is_signed = true;
  static constexpr bool is_floating = true;
  static constexpr std::string_view name = "32 bit float";

  static constexpr optional<r_texture_format> parse_channels(uint8 flags) noexcept {
    const uint8 channels = flags & TEX_DEPTH_CHANNELS_MASK;
    switch (channels) {
      case 1u: return r_texture_format::r32f;
      case 2u: return r_texture_format::rg32f;
      case 3u: return r_texture_format::rgb32f;
      case 4u: return r_texture_format::rgba32f;
    }
    return nullopt;
  }
};

template<typename T>
concept tex_array_dim_type = same_as_any<std::remove_cvref_t<T>,
  extent1d, extent2d
>;

template<typename T>
concept tex_dim_type = same_as_any<std::remove_cvref_t<T>,
  extent1d, extent2d, extent3d
>;

template<tex_dim_type D>
constexpr extent3d tex_extent_cast(const D& dim) noexcept {
  if constexpr (std::same_as<D, extent3d>) {
    return dim;
  } else if constexpr (std::same_as<D, extent2d>) {
    return extent3d{static_cast<uint32>(dim.x), static_cast<uint32>(dim.y), 1u};
  } else {
    return extent3d{static_cast<uint32>(dim), 1u, 1u};
  }
};

template<tex_dim_type D>
constexpr extent3d tex_offset_cast(const D& dim) noexcept {
  if constexpr (std::same_as<D, extent3d>) {
    return dim;
  } else if constexpr (std::same_as<D, extent2d>) {
    return extent3d{static_cast<uint32>(dim.x), static_cast<uint32>(dim.y), 0u};
  } else {
    return extent3d{static_cast<uint32>(dim), 0u, 0u};
  }
}

template<tex_depth_type T, tex_dim_type D>
constexpr size_t tex_stride(const D& extent) noexcept {
  if constexpr (std::same_as<D, extent3d>) {
    return static_cast<size_t>(extent.x*extent.y*extent.z)*sizeof(T);
  } else if constexpr (std::same_as<D, uvec2>) {
    return static_cast<size_t>(extent.x*extent.y)*sizeof(T);
  } else {
    return static_cast<size_t>(extent)*sizeof(T);
  }
};

template<tex_depth_type T, tex_dim_type D>
constexpr size_t tex_stride(T&&, const D& extent) noexcept {
  return tex_stride<T>(std::forward<D>(extent));
}

template<tex_array_dim_type D>
constexpr extent3d tex_array_offset(const D& offset, size_t i) noexcept {
  if constexpr (std::same_as<D, uvec2>) {
    return {static_cast<uint32>(offset.x), static_cast<uint32>(offset.y), static_cast<uint32>(i)};
  } else {
    return {static_cast<uint32>(offset), static_cast<uint32>(i), 0};
  }
}

class r_texture_view {
public:
  r_texture_view(r_texture tex) noexcept :
    _tex{tex} {}

public:
  r_expected<void> upload(const r_texture_data& data) const {
    return r_texture_upload(_tex, data);
  }
  void upload(unchecked_t, const r_texture_data& data) const {
    r_texture_upload(::ntf::unchecked, _tex, data);
  }
  r_expected<void> upload(cspan<r_image_data> images, bool gen_mips) const {
    return r_texture_upload(_tex, images, gen_mips);
  }
  void upload(unchecked_t, cspan<r_image_data> images, bool gen_mips) const {
    r_texture_upload(::ntf::unchecked, _tex, images, gen_mips);
  }
  r_expected<void> sampler(r_texture_sampler sampler) const {
    return r_texture_set_sampler(_tex, sampler);
  }
  void sampler(unchecked_t, r_texture_sampler sampler) const {
    r_texture_set_sampler(::ntf::unchecked, _tex, sampler);
  }
  r_expected<void> addressing(r_texture_address addressing) const {
    return r_texture_set_addressing(_tex, addressing);
  }
  void addressing(unchecked_t, r_texture_address addressing) const {
    r_texture_set_addressing(::ntf::unchecked, _tex, addressing);
  }

public:
  r_texture handle() const { return _tex; }
  r_context_view context() const { return r_texture_get_ctx(_tex); }

  r_texture_type type() const {
    return r_texture_get_type(_tex);
  }
  r_texture_format format() const {
    return r_texture_get_format(_tex);
  }
  r_texture_sampler sampler() const {
    return r_texture_get_sampler(_tex);
  }
  r_texture_address addressing() const {
    return r_texture_get_addressing(_tex);
  }
  extent3d extent() const {
    return r_texture_get_extent(_tex);
  }
  uint32 layers() const {
    return r_texture_get_layers(_tex);
  }
  uint32 levels() const {
    return r_texture_get_levels(_tex);
  }
  bool is_cubemap() const {
    return type() == r_texture_type::cubemap;
  }
  bool is_array() const {
    return !is_cubemap() && layers() > 1;
  }
  bool has_mipmaps() const {
    return levels() > 0;
  }

protected:
  r_texture _tex;
};

class renderer_texture : public r_texture_view {
private:
  struct deleter_t {
    void operator()(r_texture tex) { r_destroy_texture(tex); }
  };
  using uptr_type = std::unique_ptr<r_texture_, deleter_t>;

public:
  explicit renderer_texture(r_texture tex) noexcept :
    r_texture_view{tex},
    _handle{tex} {}

public:
  static auto create(
    r_context_view ctx, const r_texture_descriptor& desc
  ) noexcept -> r_expected<renderer_texture>
  {
    return r_create_texture(ctx.handle(), desc)
      .transform([](r_texture tex) -> renderer_texture {
        return renderer_texture{tex};
      });
  }

  static auto create(
    unchecked_t,
    r_context_view ctx, const r_texture_descriptor& desc
  ) -> renderer_texture
  {
    return renderer_texture{r_create_texture(::ntf::unchecked, ctx.handle(), desc)};
  }

private:
  uptr_type _handle;
};

} // namespace ntf
