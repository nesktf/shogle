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
concept tex_depth_type = requires() {
  (
    std::is_integral_v<typename T::underlying_type> ||
    std::is_floating_point_v<typename T::underlying_type>
  );
  { T::is_signed } -> std::convertible_to<bool>;
  { T::is_normalized } -> std::convertible_to<bool>;
  { T::is_floating } -> std::convertible_to<bool>;
  { T::is_linear } -> std::convertible_to<bool>;
  { T::name } -> std::convertible_to<std::string_view>;
  { *T::valid_channels.begin() } -> std::convertible_to<size_t>;
  { *T::valid_channels.end() } -> std::convertible_to<size_t>;
  { T::parse_channels(uint32{}) } -> same_as_any<r_texture_format, optional<r_texture_format>>;
};

struct tex_depth_u8 {
  using underlying_type = uint8;
  static constexpr bool is_signed = false;
  static constexpr bool is_normalized = false;
  static constexpr bool is_floating = false;
  static constexpr bool is_linear = true;
  static constexpr std::string_view name = "8 bit unsigned integral";
  static constexpr std::array<size_t, 4u> valid_channels{1, 2, 3, 4};

  static constexpr optional<r_texture_format> parse_channels(uint32 n) {
    switch (n) {
      case 1: return r_texture_format::r8u;
      case 2: return r_texture_format::rg8u;
      case 3: return r_texture_format::rgb8u;
      case 4: return r_texture_format::rgba8u;
    }
    return nullopt;
  }
};
static_assert(tex_depth_type<tex_depth_u8>);

struct tex_depth_u8n {
  using underlying_type = uint8;
  static constexpr bool is_signed = false;
  static constexpr bool is_normalized = true;
  static constexpr bool is_floating = false;
  static constexpr bool is_linear = true;
  static constexpr std::string_view name = "8 bit unsigned normalized";
  static constexpr std::array<size_t, 4u> valid_channels{1, 2, 3, 4};

  static constexpr optional<r_texture_format> parse_channels(uint32 n) {
    switch (n) {
      case 1: return r_texture_format::r8nu;
      case 2: return r_texture_format::rg8nu;
      case 3: return r_texture_format::rgb8nu;
      case 4: return r_texture_format::rgba8nu;
    }
    return nullopt;
  }
};
static_assert(tex_depth_type<tex_depth_u8n>);

struct tex_depth_u8nl {
  using underlying_type = uint8;
  static constexpr bool is_signed = false;
  static constexpr bool is_normalized = false;
  static constexpr bool is_floating = false;
  static constexpr bool is_linear = false;
  static constexpr std::string_view name = "8 bit unsigned nonlinear";
  static constexpr std::array<size_t, 2u> valid_channels{3, 4};

  static constexpr optional<r_texture_format> parse_channels(uint32 n) {
    switch (n) {
      case 3: return r_texture_format::srgb8u;
      case 4: return r_texture_format::srgba8u;
    }
    return nullopt;
  }
};
static_assert(tex_depth_type<tex_depth_u8nl>);

struct tex_depth_s8 {
  using underlying_type = int8;
  static constexpr bool is_signed = true;
  static constexpr bool is_normalized = false;
  static constexpr bool is_floating = false;
  static constexpr bool is_linear = true;
  static constexpr std::string_view name = "8 bit signed integral";
  static constexpr std::array<size_t, 4u> valid_channels{1, 2, 3, 4};

  static constexpr optional<r_texture_format> parse_channels(uint32 n) {
    switch (n) {
      case 1: return r_texture_format::r8i;
      case 2: return r_texture_format::rg8i;
      case 3: return r_texture_format::rgb8i;
      case 4: return r_texture_format::rgba8i;
    } 
    return nullopt;
  }
};
static_assert(tex_depth_type<tex_depth_s8>);

struct tex_depth_s8n {
  using underlying_type = int8;
  static constexpr bool is_signed = true;
  static constexpr bool is_normalized = true;
  static constexpr bool is_floating = false;
  static constexpr bool is_linear = true;
  static constexpr std::string_view name = "8 bit signed normalized";
  static constexpr std::array<size_t, 4u> valid_channels{1, 2, 3, 4};

  static constexpr optional<r_texture_format> parse_channels(uint32 n) {
    switch (n) {
      case 1: return r_texture_format::r8n;
      case 2: return r_texture_format::rg8n;
      case 3: return r_texture_format::rgb8n;
      case 4: return r_texture_format::rgba8n;
    }
    return nullopt;
  }
};
static_assert(tex_depth_type<tex_depth_s8n>);

struct tex_depth_u16 {
  using underlying_type = uint16;
  static constexpr bool is_signed = false;
  static constexpr bool is_normalized = false;
  static constexpr bool is_floating = false;
  static constexpr bool is_linear = true;
  static constexpr std::string_view name = "16 bit unsigned integral";
  static constexpr std::array<size_t, 4u> valid_channels{1, 2, 3, 4};

  static constexpr optional<r_texture_format> parse_channels(uint32 n) {
    switch (n) {
      case 1: return r_texture_format::r16u;
      case 2: return r_texture_format::rg16u;
      case 3: return r_texture_format::rgb16u;
      case 4: return r_texture_format::rgba16u;
    }
    return nullopt;
  }
};
static_assert(tex_depth_type<tex_depth_u16>);

struct tex_depth_s16 {
  using underlying_type = int16;
  static constexpr bool is_signed = true;
  static constexpr bool is_normalized = false;
  static constexpr bool is_floating = false;
  static constexpr bool is_linear = true;
  static constexpr std::string_view name = "16 bit signed integral";
  static constexpr std::array<size_t, 4u> valid_channels{1, 2, 3, 4};

  static constexpr optional<r_texture_format> parse_channels(uint32 n) {
    switch (n) {
      case 1: return r_texture_format::r16i;
      case 2: return r_texture_format::rg16i;
      case 3: return r_texture_format::rgb16i;
      case 4: return r_texture_format::rgba16i;
    }
    return nullopt;
  }
};
static_assert(tex_depth_type<tex_depth_s16>);

struct tex_depth_f32 {
  using underlying_type = float32;
  static constexpr bool is_signed = true;
  static constexpr bool is_normalized = false;
  static constexpr bool is_floating = true;
  static constexpr bool is_linear = true;
  static constexpr std::string_view name = "32 bit floating";
  static constexpr std::array<size_t, 4u> valid_channels{1, 2, 3, 4};

  static constexpr optional<r_texture_format> parse_channels(uint32 n) {
    switch (n) {
      case 1: return r_texture_format::r32f;
      case 2: return r_texture_format::rg32f;
      case 3: return r_texture_format::rgb32f;
      case 4: return r_texture_format::rgba32f;
    }
    return nullopt;
  }
};
static_assert(tex_depth_type<tex_depth_f32>);

template<typename T>
concept tex_array_dim_type = same_as_any<T,
  extent1d, extent2d
>;

template<typename T>
concept tex_array_dim_convertible = convertible_to_any<T,
  extent1d, extent2d
>;

template<typename T>
concept tex_dim_type = same_as_any<T,
  extent1d, extent2d, extent3d
>;

template<typename T>
concept tex_dim_convertible = convertible_to_any<T,
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
    return static_cast<size_t>(extent.x*extent.y*extent.z)*sizeof(T::underlying_type);
  } else if constexpr (std::same_as<D, uvec2>) {
    return static_cast<size_t>(extent.x*extent.y)*sizeof(T::underlying_type);
  } else {
    return static_cast<size_t>(extent)*sizeof(T::underlying_type);
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
