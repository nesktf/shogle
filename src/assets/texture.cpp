#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "./texture.hpp"

namespace ntf {

namespace {

template<typename T, bool>
struct stbi_load_ret {
  using type = asset_expected<stb_image_loader::stbi_ptr<T>>;
};

template<typename T>
struct stbi_load_ret<T, false> {
  using type = stb_image_loader::stbi_ptr<T>;
};

template<typename T, bool checked>
auto stbi_load_impl(const std::string& path,
                    uint32& w, uint32& h, uint32& ch,
                    image_load_flags flags) -> stbi_load_ret<T, checked>::type {
  int w_, h_, ch_;
  stbi_set_flip_vertically_on_load(+(flags & image_load_flags::flip_vertically));

  T* stbi_data;
  if constexpr (std::same_as<T, uint8>) {
    stbi_data = stbi_load(path.c_str(), &w_, &h_, &ch_, 0);
  } else if constexpr (std::same_as<T, uint16>) {
    stbi_data = stbi_load_16(path.c_str(), &w_, &h_, &ch_, 0);
  } else if constexpr (std::same_as<T, float32>) {
    stbi_data = stbi_loadf(path.c_str(), &w_, &h_, &ch_, 0);
  } else {
    NTF_UNREACHABLE();
  }

  if constexpr (checked) {
    if (!stbi_data) {
      const char* err = stbi_failure_reason();
      SHOGLE_LOG(error, "[ntf::stb_image_loader] Failed to load image \"{}\": {}", path, err);
      return unexpected{asset_error::format({"{}"}, err)};
    }
  } else {
    NTF_ASSERT(stbi_data);
  }

  w = static_cast<uint32>(w_);
  h = static_cast<uint32>(h_);
  ch = static_cast<uint32>(ch_);

  SHOGLE_LOG(debug, "[ntf::stb_image_loader] Loaded {} {} {}x{} image \"{}\"",
             impl::img_depth_str<T>::value, impl::img_RGBA_str[ch], w, h, path);

  return stb_image_loader::stbi_ptr<T>{stbi_data, stb_image_loader::deleter<T>{}};
}

} // namespace

void stb_image_loader::stbi_delete(void* data) {
  stbi_image_free(data);
}

auto stb_image_loader::load_rgb8u(const std::string& path,
                                  uint32& w, uint32& h, uint32& ch,
                                  image_load_flags flags) -> asset_expected<stbi_ptr<uint8>> {
  return stbi_load_impl<uint8, true>(path, w, h, ch, flags);
}

auto stb_image_loader::load_rgb8u(unchecked_t,
                                  const std::string& path,
                                  uint32& w, uint32& h, uint32& ch,
                                  image_load_flags flags) -> stbi_ptr<uint8> {
  return stbi_load_impl<uint8, false>(path, w, h, ch, flags);
}

auto stb_image_loader::load_rgb16u(const std::string& path,
                                   uint32& w, uint32& h, uint32& ch,
                                   image_load_flags flags) -> asset_expected<stbi_ptr<uint16>> {
  return stbi_load_impl<uint16, true>(path, w, h, ch, flags);
}

auto stb_image_loader::load_rgb16u(unchecked_t,
                                   const std::string& path,
                                   uint32& w, uint32& h, uint32& ch,
                                   image_load_flags flags) -> stbi_ptr<uint16> {
  return stbi_load_impl<uint16, false>(path, w, h, ch, flags);
}

auto stb_image_loader::load_rgb32f(const std::string& path,
                                   uint32& w, uint32& h, uint32& ch,
                                   image_load_flags flags) -> asset_expected<stbi_ptr<float32>> {
  return stbi_load_impl<float32, true>(path, w, h, ch, flags);
}

auto stb_image_loader::load_rgb32f(unchecked_t,
                                   const std::string& path,
                                   uint32& w, uint32& h, uint32& ch,
                                   image_load_flags flags) -> stbi_ptr<float32> {
  return stbi_load_impl<float32, false>(path, w, h, ch, flags);
}

} // namespace ntf
