#pragma once

#include <shogle/render/texture.hpp>

#include <shogle/core/error.hpp>

#include <shogle/res/util.hpp>

#include <nlohmann/json.hpp>

#include <stb/stb_image.h>

namespace ntf {

namespace impl {

constexpr tex_format toenum(int channels) {
  switch (channels) {
    case 1:
      return tex_format::mono;
    case 4:
      return tex_format::rgba;
    default:
      return tex_format::rgb;
  }
}

} // namespace impl

struct texture2d_data {
public:
  inline texture2d_data(std::string_view path_);

public:
  uint8_t* pixels{};
  size_t width{}, height{};
  tex_format format{};

public:
  inline ~texture2d_data();
  inline texture2d_data(texture2d_data&&) noexcept;
  texture2d_data(const texture2d_data&) = delete;
  inline texture2d_data& operator=(texture2d_data&&) noexcept;
  texture2d_data& operator=(const texture2d_data&) = delete;

private:
  friend struct spritesheet_data;
  texture2d_data() = default;
};

struct cubemap_data {
public:
  inline cubemap_data(std::string_view path_);

public:
  cmappixels pixels{};
  size_t dim{};
  tex_format format{};

public:
  inline ~cubemap_data();
  inline cubemap_data(cubemap_data&& c) noexcept;
  cubemap_data(const cubemap_data&) = delete;
  inline cubemap_data& operator=(cubemap_data&&) noexcept;
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

texture2d_data::texture2d_data(std::string_view path_) {
  int w, h, ch;
  pixels = stbi_load(path_.data(), &w, &h, &ch, 0);
  if (!pixels) {
    throw ntf::error{"[shogle::texture_data] Error loading texture: {}", path_};
  }

  width = static_cast<size_t>(w);
  height = static_cast<size_t>(h);
  format = impl::toenum(ch);
}

texture2d_data::~texture2d_data() {
  if (pixels) {
    stbi_image_free(pixels);
  }
}

texture2d_data::texture2d_data(texture2d_data&& t) noexcept :
  pixels(std::move(t.pixels)),
  width(std::move(t.width)), height(std::move(t.height)),
  format(std::move(t.format)) {
  t.pixels = nullptr;
}

texture2d_data& texture2d_data::operator=(texture2d_data&& t) noexcept {
  if (pixels) {
    stbi_image_free(pixels);
  }

  pixels = std::move(t.pixels);
  width = std::move(t.width);
  height = std::move(t.height);
  format = std::move(t.format);

  t.pixels = nullptr;

  return *this;
}

cubemap_data::cubemap_data(std::string_view path_) {
  using json = nlohmann::json;
  std::ifstream f{path_.data()};
  json data = json::parse(f);

  auto content = data["content"];
  assert(content.size() == SHOGLE_CUBEMAP_FACES && "Invalid json data");

  int w, h, ch;
  for (size_t i = 0; i < content.size(); ++i) {
    auto& curr = content[i];

    std::string path = file_dir(path_.data())+"/"+curr["path"].template get<std::string>();
    pixels[i] = stbi_load(path.c_str(), &w, &h, &ch, 0);

    if (!pixels[i]) {
      throw ntf::error{"[shogle::cubemap_data] Error loading cubemap texture: {}, n: {}", path, i};
    }
  }

  // leeches off the last texture
  // all of them >should< have the same format
  dim = static_cast<size_t>(w);
  format = impl::toenum(ch);
}

cubemap_data::~cubemap_data() {
  for (auto& curr : pixels) {
    if (curr) {
      stbi_image_free(curr);
    }
  }
}

cubemap_data::cubemap_data(cubemap_data&& c) noexcept :
  pixels(std::move(c.pixels)),
  dim(std::move(c.dim)),
  format(std::move(c.format)) {
  for (auto& curr : c.pixels) {
    curr = nullptr;
  }
}

cubemap_data& cubemap_data::operator=(cubemap_data&& c) noexcept {
  for (auto& curr : pixels) {
    if (curr) {
      stbi_image_free(curr);
    }
  }

  pixels = std::move(c.pixels);
  dim = std::move(c.dim);
  format = std::move(c.format);

  for (auto& curr : c.pixels) {
    curr = nullptr;
  }

  return *this;
}

} // namespace ntf
