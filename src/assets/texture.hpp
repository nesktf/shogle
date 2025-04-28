#pragma once

#include "./types.hpp"
#include "./filesystem.hpp"

#include "../render/texture.hpp"

namespace ntf {

struct image_data {
public:
  using texel_array = unique_array<uint8, virtual_array_deleter<uint8>>;

public:
  image_data(texel_array&& texels_, extent3d extent_,
             r_texture_format format_, r_image_alignment alignment_) noexcept :
    texels{std::move(texels_)}, extent{extent_},
    format{format_}, alignment{alignment_} {}

  image_data(texel_array&& texels_, extent2d extent_,
             r_texture_format format_, r_image_alignment alignment_) noexcept :
    texels{std::move(texels_)}, extent{extent_.x, extent_.y, 1u},
    format{format_}, alignment{alignment_} {}

  image_data(texel_array&& texels_, extent1d extent_,
             r_texture_format format_, r_image_alignment alignment_) noexcept :
    texels{std::move(texels_)}, extent{extent_, 1u, 1u},
    format{format_}, alignment{alignment_} {}

public:
  template<tex_dim_type DimT = extent3d>
  r_image_data make_descriptor(
    const DimT& offset = {}, uint32 level = 0, uint32 layer = 0
  ) const noexcept {
    return {
      .texels = texels.get(),
      .format = format,
      .alignment = alignment,
      .extent = extent,
      .offset = tex_extent_cast(offset),
      .layer = layer,
      .level = level,
    };
  }

public:
  texel_array texels;
  extent3d extent;
  r_texture_format format;
  r_image_alignment alignment;
};

struct cubemap_data {
public:
  using texel_array = unique_array<uint8, virtual_array_deleter<uint8>>;

public:
  cubemap_data(std::array<texel_array, 6u>&& texels_, extent1d extent_,
               r_texture_format format_, r_image_alignment alignment_) noexcept :
    texels{std::move(texels_)}, extent{extent_, extent_},
    format{format_}, alignment{alignment_} {}

public:
  std::array<r_image_data, 6u> make_descriptor(
    const extent2d& offset = {}, uint32 level = 0
  ) const noexcept {
    std::array<r_image_data, 6u> images;

    for (size_t i = 0; auto& image : images) {
      image.texels = texels[i].get();
      image.format = format;
      image.alignment = alignment;
      image.extent = tex_extent_cast(extent);
      image.offset = tex_array_offset(offset, i);
      image.layer = static_cast<uint32>(i);
      image.level = level;
      ++i;
    }

    return images;
  }

public:
  std::array<texel_array, 6u> texels;
  extent2d extent;
  r_texture_format format;
  r_image_alignment alignment;
};

enum class image_load_flags {
  none = 0,
  flip_y = 1 << 0,
  mark_normalized = 1 << 1,
  mark_nonlinear = 1 << 2,
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(image_load_flags);

class stb_image_loader {
private:
  friend struct stbi_deleter;
  struct stbi_deleter {
    void operator()(uint8* data, size_t) noexcept {
      stb_image_loader::_stbi_delete(data);
    };
  };

  enum image_format {
    STBI_FORMAT_U8 = 0,
    STBI_FORMAT_U16,
    STBI_FORMAT_F32,
  };

  struct stbi_data {
    uint8* data;
    uint32 width;
    uint32 height;
    uint32 channels;
  };

public:
  stb_image_loader() noexcept = default;

public:
  template<typename DepthT>
  requires(same_as_any<DepthT, uint8, uint16, float>)
  asset_expected<image_data> load_image(
    cspan<uint8> file_data, image_load_flags flags, uint32 channels
  ) {
    constexpr image_format stbi_format = [](){
      if constexpr (std::same_as<DepthT, uint8>) {
        return STBI_FORMAT_U8;
      } else if constexpr (std::same_as<DepthT, uint16>) {
        return STBI_FORMAT_U16;
      } else {
        return STBI_FORMAT_F32;
      }
    }();

    return _load_image(file_data, channels, +(flags & image_load_flags::flip_y), stbi_format)
    .and_then([flags](stbi_data&& image) -> asset_expected<image_data> {
      const uint8 channel_flags =
        (+(flags & image_load_flags::mark_nonlinear) ? TEX_DEPTH_NONLINEAR_BIT : 0) |
        (+(flags & image_load_flags::mark_normalized) ? TEX_DEPTH_NORMALIZE_BIT : 0) |
        (image.channels & TEX_DEPTH_CHANNELS_MASK);

      auto format = tex_depth_traits<DepthT>::parse_channels(channel_flags);
      if (!format) {
        _stbi_delete(image.data);
        return unexpected{asset_error{"Failed to parse channel tag"}};
      }
      SHOGLE_LOG(debug, "[ntf::stb_image_loader] Loaded {} {}x{} image with {} channels",
                 tex_depth_traits<DepthT>::name, image.width, image.height, image.channels);

      const size_t arr_sz = image.channels*image.height*image.width*sizeof(DepthT);
      return image_data{
        image_data::texel_array{image.data, arr_sz, virtual_array_deleter<uint8>{stbi_deleter{}}},
        extent2d{image.width, image.height}, *format, r_image_alignment::bytes4
      };
    });
  }

  asset_expected<image_data> load_image(
    tex_depth_traits<uint8>::tag_type,
    cspan<uint8> file_data, image_load_flags flags, uint32 channels
  ) {
    return load_image<uint8>(file_data, flags, channels);
  }

  asset_expected<image_data> load_image(
    tex_depth_traits<uint16>::tag_type,
    cspan<uint8> file_data, image_load_flags flags, uint32 channels
  ) {
    return load_image<uint16>(file_data, flags, channels);
  }

  asset_expected<image_data> load_image(
    tex_depth_traits<float>::tag_type,
    cspan<uint8> file_data, image_load_flags flags, uint32 channels
  ) {
    return load_image<float>(file_data, flags, channels);
  }

private:
  asset_expected<stbi_data> _load_image(cspan<uint8> file_data, uint32 channels,
                                        bool flip_y, image_format format);

public:
  static optional<std::pair<extent2d, uint32>> parse_image(const std::string& file);
  static optional<std::pair<extent2d, uint32>> parse_image(std::FILE* file);
  static optional<std::pair<extent2d, uint32>> parse_image(cspan<uint8> file_data);

private:
  static void _stbi_delete(void* data) noexcept;
};

template<tex_depth_type DepthT, typename Loader = stb_image_loader>
asset_expected<image_data> load_image(
  const std::string& path,
  image_load_flags flags = image_load_flags::mark_normalized | image_load_flags::flip_y,
  uint32 channels = 0u, Loader&& loader = {}
) {
  SHOGLE_LOG(debug, "[ntf::load_image] Loading image from file '{}'", path);
  return file_data(path)
    .and_then([&](auto&& buffer) {
      return loader.load_image(tex_depth_traits<DepthT>::tag,
                               cspan<uint8>{buffer.get(), buffer.size()}, flags, channels);
    });
}

} // namespace ntf
