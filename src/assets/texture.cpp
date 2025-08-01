#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <shogle/assets/texture.hpp>

namespace ntf {

optional<std::pair<ntfr::extent2d, uint32>> stb_image_loader::parse_image(const std::string& path) {
  int w, h, ch;
  if (stbi_info(path.c_str(), &w, &h, &ch)) {
    return std::make_pair(
      ntfr::extent2d{static_cast<uint32>(w), static_cast<uint32>(h)}, static_cast<uint32>(ch)
    );
  }
  return nullopt;
}

optional<std::pair<ntfr::extent2d, uint32>> stb_image_loader::parse_image(std::FILE* f) {
  int w, h, ch;
  if (stbi_info_from_file(f, &w, &h, &ch)) {
    return std::make_pair(
      ntfr::extent2d{static_cast<uint32>(w), static_cast<uint32>(h)}, static_cast<uint32>(ch)
    );
  }
  return nullopt;
}

optional<std::pair<ntfr::extent2d, uint32>> stb_image_loader::parse_image(cspan<uint8> file_data) {
  int w, h, ch;
  if (stbi_info_from_memory(file_data.data(), file_data.size(), &w, &h, &ch)) {
    return std::make_pair(
      ntfr::extent2d{static_cast<uint32>(w), static_cast<uint32>(h)}, static_cast<uint32>(ch)
    );
  }
  return nullopt;
}

auto stb_image_loader::_load_image(cspan<uint8> file_data, uint32 channels,
                                   bool flip_y, image_format format) -> asset_expected<stbi_data>
{
  stbi_set_flip_vertically_on_load(flip_y);
  int w, h, ch;
  uint8* data = nullptr;
  switch (format) {
    case stb_image_loader::STBI_FORMAT_U8: {
      SHOGLE_LOG(verbose, "[ntf::stb_image_loader] Loading image from {} in u8 format",
                 fmt::ptr(file_data.data()));
      data = reinterpret_cast<uint8*>(stbi_load_from_memory(
        file_data.data(), file_data.size_bytes(),
        &w, &h, &ch, static_cast<int>(channels)
      ));
      break;
    }
    case stb_image_loader::STBI_FORMAT_U16: {
      SHOGLE_LOG(verbose, "[ntf::stb_image_loader] Loading image from {} in u16 format",
                 fmt::ptr(file_data.data()));
      data = reinterpret_cast<uint8*>(stbi_load_16_from_memory(
        file_data.data(), file_data.size_bytes(),
        &w, &h, &ch, static_cast<int>(channels)
      ));
      break;
    }
    case stb_image_loader::STBI_FORMAT_F32: {
      SHOGLE_LOG(verbose, "[ntf::stb_image_loader] Loading image from {} in f32 format",
                 fmt::ptr(file_data.data()));
      data = reinterpret_cast<uint8*>(stbi_loadf_from_memory(
        file_data.data(), file_data.size_bytes(),
        &w, &h, &ch, static_cast<int>(channels)
      ));
      break;
    }
  }
  if (!data) {
    SHOGLE_LOG(error, "[ntf::stb_image_loader] Failed to parse image: {}", stbi_failure_reason());
    return unexpected{asset_error{stbi_failure_reason()}};
  }
  SHOGLE_LOG(verbose, "[ntf::stb_image_loader] Parsed {} channels (marked {})",
             ch, channels);
  return stbi_data {
    .data = data,
    .width = static_cast<uint32>(w),
    .height = static_cast<uint32>(h),
    .channels = static_cast<uint32>(ch),
  };
}

void stb_image_loader::_stbi_delete(void* data) noexcept {
  stbi_image_free(data);
}

} // namespace ntf
