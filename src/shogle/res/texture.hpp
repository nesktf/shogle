#pragma once

#include <shogle/render/texture.hpp>

namespace ntf::shogle {

struct texture2d_data {
public:
  texture2d_data(std::string_view path_, tex_filter filter_, tex_wrap wrap_);

public:
  uint8_t* pixels{};
  size_t width{}, height{};
  tex_format format{};
  tex_filter filter{};
  tex_wrap wrap{};

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
  cubemap_data(std::string_view path_, tex_filter filter_, tex_wrap wrap_);

public:
  std::array<uint8_t*, CUBEMAP_FACES> pixels{};
  size_t dim{};
  tex_format format{};
  tex_filter filter{};
  tex_wrap wrap{};

public:
  ~cubemap_data();
  cubemap_data(cubemap_data&& c) noexcept;
  cubemap_data(const cubemap_data&) = delete;
  cubemap_data& operator=(cubemap_data&&) noexcept;
  cubemap_data& operator=(const cubemap_data&) = delete;
};

texture2d load_texture2d(std::string_view path, tex_filter filter, tex_wrap wrap);
texture2d load_texture2d(texture2d_data data);

cubemap load_cubemap(std::string_view path, tex_filter filter, tex_wrap wrap);
cubemap load_cubemap(cubemap_data data);

} // namespace ntf::shogle
