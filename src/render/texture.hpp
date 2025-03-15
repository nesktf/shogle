#pragma once

#include "./forward.hpp"
#include "../stl/optional.hpp"

namespace ntf {

using extent1d = uint32;
using extent2d = uvec2;
using extent3d = uvec3;

SHOGLE_DECLARE_RENDER_HANDLE(r_texture_handle);

enum class r_texture_type : uint8 {
  texture1d = 0,
  texture2d,
  texture3d,
  cubemap,
};

enum class r_texture_format : uint8 {
  r8nu=0,  r8n,     r8u,     r8i,
  r16u,    r16i,    r16f,
  r32u,    r32i,    r32f,

  rg8nu,   rg8n,    rg8u,    rg8i,
  rg16u,   rg16i,   rg16f,
  rg32u,   rg32i,   rg32f,

  rgb8nu,  rgb8n,   rgb8u,   rgb8i,
  rgb16u,  rgb16i,  rgb16f,
  rgb32u,  rgb32i,  rgb32f,

  rgba8nu, rgba8n,  rgba8u,  rgba8i,
  rgba16u, rgba16i, rgba16f,
  rgba32u, rgba32i, rgba32f,

  srgb8u,  srgba8u,
};

enum class r_texture_sampler : uint8 {
  nearest = 0,
  linear,
};

enum class r_texture_address : uint8 {
  repeat = 0,
  repeat_mirrored,
  clamp_edge,
  clamp_edge_mirrored,
  clamp_border,
};

enum class r_cubemap_face : uint8 {
  positive_x = 0,
  negative_x,
  positive_y,
  negative_y,
  positive_z,
  negative_z,
};
constexpr inline uint32 r_cubemap_layer(r_cubemap_face face) {
  return static_cast<uint32>(face);
}

enum class r_image_alignment : uint8 {
  bytes1 = 1,
  bytes2 = 2,
  bytes4 = 4,
  bytes8 = 8,
};

struct r_image_data {
  const void* texels;
  r_texture_format format;
  r_image_alignment alignment;

  extent3d extent;
  extent3d offset;

  uint32 layer;
  uint32 level;
};

struct r_texture_descriptor {
  r_texture_type type;
  r_texture_format format;

  extent3d extent;
  uint32 layers;
  uint32 levels;

  span_view<r_image_data> images;
  bool gen_mipmaps{false};
  r_texture_sampler sampler;
  r_texture_address addressing;
};

struct r_texture_data {
  span_view<r_image_data> images;
  bool gen_mipmaps{false};
  optional<r_texture_sampler> sampler;
  optional<r_texture_address> addressing;
};

struct r_texture_binding {
  r_texture_handle texture;
  uint32 location;
};

template<typename T>
concept tex_depth_type = same_as_any<std::remove_cvref_t<T>,
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

} // namespace ntf
