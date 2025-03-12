#pragma once

#include "./forward.hpp"
#include "../stl/optional.hpp"

namespace ntf {

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

  uvec3 extent;
  uvec3 offset;

  uint32 layer;
  uint32 level;
};

struct r_texture_descriptor {
  r_texture_type type;
  r_texture_format format;

  uvec3 extent;
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

NTF_DECLARE_TAG_TYPE(uint8n);  // uint8 normalized
NTF_DECLARE_TAG_TYPE(int8n);   // int8 normalized
NTF_DECLARE_TAG_TYPE(uint8nl); // uint8 nonlinear

template<typename T>
concept image_array_dim_type = same_as_any<T, uint32, uvec2>;

template<typename T>
concept image_dim_type = image_array_dim_type<T> || std::same_as<T, uvec3>;

template<typename T>
concept image_depth_type = same_as_any<T, 
  uint8n_t, uint8nl_t, int8n_t,
  uint8, int8,
  uint16, int16,
  float32
>;

constexpr std::array<std::string_view, 4u> image_depth_RGBA_str = { "R", "RG", "RGB", "RGBA" };

template<image_depth_type T>
struct image_depth_traits;

template<>
struct image_depth_traits<uint8n_t> {
  using underlying_t = uint8;
  static constexpr std::string_view name = "8 bit unsigned normalized";
  static constexpr std::array<size_t, 4u> channels{1, 2, 3, 4};
  static constexpr size_t bytes = 1;

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

template<>
struct image_depth_traits<uint8nl_t> {
  using underlying_t = uint8;
  static constexpr std::string_view name = "8 bit unsigned nonlinear";
  static constexpr std::array<size_t, 4u> channels{3, 4};
  static constexpr size_t bytes = 1;

  static constexpr optional<r_texture_format> parse_channels(uint32 n) {
    switch (n) {
      case 3: return r_texture_format::srgb8u;
      case 4: return r_texture_format::srgba8u;
    }
    return nullopt;
  }
};

template<>
struct image_depth_traits<int8n_t> {
  using underlying_t = int8;
  static constexpr std::string_view name = "8 bit signed normalized";
  static constexpr std::array<size_t, 4u> channels{1, 2, 3, 4};
  static constexpr size_t bytes = 1;

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

template<>
struct image_depth_traits<uint8> {
  using underlying_t = uint8;
  static constexpr std::string_view name = "8 bit unsigned integral";
  static constexpr std::array<size_t, 4u> channels{1, 2, 3, 4};
  static constexpr size_t bytes = 1;

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

template<>
struct image_depth_traits<int8> {
  using underlying_t = int8;
  static constexpr std::string_view name = "8 bit signed integral";
  static constexpr std::array<size_t, 4u> channels{1, 2, 3, 4};
  static constexpr size_t bytes = 1;

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

template<>
struct image_depth_traits<uint16> {
  using underlying_t = uint16;
  static constexpr std::string_view name = "16 bit unsigned integral";
  static constexpr std::array<size_t, 4u> channels{1, 2, 3, 4};
  static constexpr size_t bytes = 2;

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

template<>
struct image_depth_traits<int16> {
  using underlying_t = int16;
  static constexpr std::string_view name = "16 bit signed integral";
  static constexpr std::array<size_t, 4u> channels{1, 2, 3, 4};
  static constexpr size_t bytes = 2;

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

template<>
struct image_depth_traits<float32> {
  using underlying_t = float32;
  static constexpr std::string_view name = "32 bit float";
  static constexpr std::array<size_t, 4u> channels{1, 2, 3, 4};
  static constexpr size_t bytes = 4;

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

template<image_dim_type T>
constexpr optional<r_texture_format> parse_channel_depth(uint32 n) {
  return image_depth_traits<T>::parse_channels(n);
}

template<image_dim_type Dim>
constexpr uvec3 image_dim_cast(const Dim& dim) noexcept {
  if constexpr(std::same_as<Dim, uvec3>) {
    return dim;
  } else if constexpr(std::same_as<Dim, uvec2>) {
    return uvec3{static_cast<uint32>(dim.x), static_cast<uint32>(dim.y), 0};
  } else {
    return uvec3{static_cast<uint32>(dim), 0, 0};
  }
}

template<image_array_dim_type Dim>
constexpr uvec3 image_array_offset_cast(const Dim& offset, size_t i) noexcept {
  if constexpr (std::same_as<Dim, uvec2>) {
    return {static_cast<uint32>(offset.x), static_cast<uint32>(offset.y), static_cast<uint32>(i)};
  } else {
    return {static_cast<uint32>(offset), static_cast<uint32>(i), 0};
  }
}

} // namespace ntf
