#pragma once

#include "./types.hpp"
#include "./filesystem.hpp"

#include <shogle/render/texture.hpp>

namespace ntf {

struct bitmap_data {
public:
  using texel_array = unique_array<uint8, allocator_delete<uint8, virtual_allocator<uint8>>>;

public:
  bitmap_data(texel_array&& texels_, ntfr::extent3d extent_,
             ntfr::image_format format_, ntfr::image_alignment alignment_) noexcept :
    texels{std::move(texels_)}, extent{extent_},
    format{format_}, alignment{alignment_} {}

  bitmap_data(texel_array&& texels_, ntfr::extent2d extent_,
             ntfr::image_format format_, ntfr::image_alignment alignment_) noexcept :
    texels{std::move(texels_)}, extent{extent_.x, extent_.y, 1u},
    format{format_}, alignment{alignment_} {}

  bitmap_data(texel_array&& texels_, ntfr::extent1d extent_,
             ntfr::image_format format_, ntfr::image_alignment alignment_) noexcept :
    texels{std::move(texels_)}, extent{extent_, 1u, 1u},
    format{format_}, alignment{alignment_} {}

public:
  template<meta::image_dim_type T = ntfr::extent3d>
  ntfr::image_data make_descriptor(
    const T& offset = {}, uint32 level = 0, uint32 layer = 0
  ) const noexcept {
    return {
      .bitmap = texels.get(),
      .format = format,
      .alignment = alignment,
      .extent = extent,
      .offset = ntfr::image_offset_cast(offset),
      .layer = layer,
      .level = level,
    };
  }

public:
  texel_array texels;
  ntfr::extent3d extent;
  ntfr::image_format format;
  ntfr::image_alignment alignment;
};

struct cubemap_data {
public:
  using texel_array = unique_array<uint8, allocator_delete<uint8, virtual_allocator<uint8>>>;

public:
  cubemap_data(std::array<texel_array, 6u>&& texels_, ntfr::extent1d extent_,
               ntfr::image_format format_, ntfr::image_alignment alignment_) noexcept :
    texels{std::move(texels_)}, extent{extent_, extent_},
    format{format_}, alignment{alignment_} {}

public:
  std::array<ntfr::image_data, 6u> make_descriptor(
    const ntfr::extent2d& offset = {}, uint32 level = 0
  ) const noexcept {
    std::array<ntfr::image_data, 6u> images;

    for (size_t i = 0; auto& image : images) {
      image.bitmap = texels[i].get();
      image.format = format;
      image.alignment = alignment;
      image.extent = ntfr::image_extent_cast(extent);
      image.offset = ntfr::image_offset_cast(offset, i);
      image.layer = static_cast<uint32>(i);
      image.level = level;
      ++i;
    }

    return images;
  }

public:
  std::array<texel_array, 6u> texels;
  ntfr::extent2d extent;
  ntfr::image_format format;
  ntfr::image_alignment alignment;
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
    void* allocate(size_t size, size_t align) {
      NTF_UNUSED(size);
      NTF_UNUSED(align);
      NTF_UNREACHABLE();
    }
    void deallocate(void* mem, size_t size) noexcept {
      NTF_UNUSED(size);
      stb_image_loader::_stbi_delete(mem);
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
  requires(meta::same_as_any<DepthT, uint8, uint16, float>)
  asset_expected<bitmap_data> load_image(
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
    .and_then([flags](stbi_data&& image) -> asset_expected<bitmap_data> {
      const uint8 channel_flags =
        (+(flags & image_load_flags::mark_nonlinear) ? ntfr::IMAGE_DEPTH_NONLINEAR_BIT : 0) |
        (+(flags & image_load_flags::mark_normalized) ? ntfr::IMAGE_DEPTH_NORMALIZE_BIT : 0) |
        (image.channels & ntfr::IMAGE_DEPTH_CHANNELS_MASK);

      auto format = meta::image_depth_traits<DepthT>::parse_channels(channel_flags);
      if (!format) {
        _stbi_delete(image.data);
        return unexpected{asset_error{"Failed to parse channel tag"}};
      }
      SHOGLE_LOG(debug, "[ntf::stb_image_loader] Loaded {} {}x{} image with {} channels",
                 meta::image_depth_traits<DepthT>::name, image.width, image.height, image.channels);

      const size_t arr_sz = image.channels*image.height*image.width*sizeof(DepthT);

      virtual_allocator<uint8> stbi_alloc{std::in_place_type_t<stbi_deleter>{}};
      return bitmap_data{
        bitmap_data::texel_array{arr_sz, image.data,
          allocator_delete<uint8, virtual_allocator<uint8>>{std::move(stbi_alloc)}
        },
        ntfr::extent2d{image.width, image.height}, *format, 4u // alignment = 4 bytes
      };
    });
  }

  asset_expected<bitmap_data> load_image(
    meta::image_depth_traits<uint8>::tag_type,
    cspan<uint8> file_data, image_load_flags flags, uint32 channels
  ) {
    return load_image<uint8>(file_data, flags, channels);
  }

  asset_expected<bitmap_data> load_image(
    meta::image_depth_traits<uint16>::tag_type,
    cspan<uint8> file_data, image_load_flags flags, uint32 channels
  ) {
    return load_image<uint16>(file_data, flags, channels);
  }

  asset_expected<bitmap_data> load_image(
    meta::image_depth_traits<float>::tag_type,
    cspan<uint8> file_data, image_load_flags flags, uint32 channels
  ) {
    return load_image<float>(file_data, flags, channels);
  }

private:
  asset_expected<stbi_data> _load_image(cspan<uint8> file_data, uint32 channels,
                                        bool flip_y, image_format format);

public:
  static optional<std::pair<ntfr::extent2d, uint32>> parse_image(const std::string& file);
  static optional<std::pair<ntfr::extent2d, uint32>> parse_image(std::FILE* file);
  static optional<std::pair<ntfr::extent2d, uint32>> parse_image(cspan<uint8> file_data);

private:
  static void _stbi_delete(void* data) noexcept;
};

template<meta::image_depth_type DepthT, typename Loader = stb_image_loader>
asset_expected<bitmap_data> load_image(
  const std::string& path,
  image_load_flags flags = image_load_flags::mark_normalized | image_load_flags::flip_y,
  uint32 channels = 0u, Loader&& loader = {}
) {
  SHOGLE_LOG(debug, "[ntf::load_image] Loading image from file '{}'", path);
  return file_data(path)
    .and_then([&](auto&& buffer) {
      return loader.load_image(meta::image_depth_traits<DepthT>::tag,
                               cspan<uint8>{buffer.get(), buffer.size()}, flags, channels);
    });
}

} // namespace ntf
