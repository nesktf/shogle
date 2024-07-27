#pragma once

#include <shogle/render/texture.hpp>

namespace ntf::shogle {

struct texture2d_data {
public:
  texture2d_data(std::string_view path_);

public:
  uint8_t* pixels{};
  size_t width{}, height{};
  tex_format format{};

public:
  ~texture2d_data();
  texture2d_data(texture2d_data&&) noexcept;
  texture2d_data(const texture2d_data&) = delete;
  texture2d_data& operator=(texture2d_data&&) noexcept;
  texture2d_data& operator=(const texture2d_data&) = delete;

private:
  friend struct spritesheet_data;
  texture2d_data() = default;
};

struct cubemap_data {
public:
  cubemap_data(std::string_view path_);

public:
  cmappixels pixels{};
  size_t dim{};
  tex_format format{};

public:
  ~cubemap_data();
  cubemap_data(cubemap_data&& c) noexcept;
  cubemap_data(const cubemap_data&) = delete;
  cubemap_data& operator=(cubemap_data&&) noexcept;
  cubemap_data& operator=(const cubemap_data&) = delete;
};

inline texture2d load_texture(std::string_view path, tex_filter filter, tex_wrap wrap) {
  auto data = texture2d_data{path};
  return load_texture(data.pixels, data.width, data.height, data.format, filter, wrap);
}

inline cubemap load_cubemap(std::string_view path, tex_filter filter, tex_wrap wrap) {
  auto data = cubemap_data{path};
  return load_cubemap(data.pixels, data.dim, data.format, filter, wrap);
}

} // namespace ntf::shogle
