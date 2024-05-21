#include <shogle/resources/texture.hpp>

#include <shogle/resources/util.hpp>

#include <shogle/core/error.hpp>
#include <shogle/core/log.hpp>

#include <stb/stb_image.h>

#include <nlohmann/json.hpp>

#include <fstream>

namespace ntf::shogle::resources {

texture2d_data::texture2d_data(std::string _path) :
  path(std::move(_path)) {
  pixels = stbi_load(path.c_str(), &width, &height, &channels, 0);
  if (!pixels) {
    throw ntf::error{"[resources::texture_data] Error loading texture: {}", path};
  }
  switch (channels) {
    case 1: {
      format = gl::texture::format::mono;
      break;
    }
    case 4: {
      format = gl::texture::format::rgba;
      break;
    };
    default: {
      format = gl::texture::format::rgb;
      break;
    }
  }
  Log::verbose("[resources::texture_data] Texture data loaded (path: {})", path);
}

texture2d_data::~texture2d_data() {
  if (pixels) {
    stbi_image_free(pixels);
  }
  Log::verbose("[resources::texture_data] Texture data unloaded (path: {})", path);
}

texture2d_data::texture2d_data(texture2d_data&& t) noexcept :
  width(t.width), height(t.height), channels(t.channels),
  pixels(t.pixels) { t.pixels = nullptr; }

texture2d_data& texture2d_data::operator=(texture2d_data&& t) noexcept {
  if (pixels) {
    Log::verbose("[resources::texture_data] Texture data overwritten (path: {} -> {})", path, t.path);
    stbi_image_free(pixels);
  }

  path = std::move(t.path);
  width = t.width;
  height = t.height;
  channels = t.channels;
  pixels = t.pixels;

  t.pixels = 0;

  return *this;
}

cubemap_data::cubemap_data(std::string _path) :
  path(std::move(_path)) {

  using json = nlohmann::json;
  std::ifstream f{path};

  json data = json::parse(f);
  auto content = data["content"];
  assert(content.size() == CUBEMAP_FACES);

  size_t i = 0;
  for (auto& curr : content) {
    std::string curr_path = file_dir(path)+"/"+curr["path"].template get<std::string>();

    // assumes all 6 faces have the same size
    pixels[i] = stbi_load(curr_path.c_str(), &width, &height, &channels, 0);
    if (!pixels[i]) {
      throw ntf::error{"[resources::cubemap_data] Error loading cubemap texture: {}, n° {}", path, i};
    }

    ++i;
  }

  switch (channels) {
    case 1: {
      format = gl::texture::format::mono;
      break;
    }
    case 4: {
      format = gl::texture::format::rgba;
      break;
    };
    default: {
      format = gl::texture::format::rgb;
      break;
    }
  }

  Log::verbose("[resources::cubemap_data] Cubemap data loaded (path: {})", path);
}

cubemap_data::~cubemap_data() {
  bool skip_log{true};
  for (size_t i = 0; i < CUBEMAP_FACES; ++i) {
    if (pixels[i]) {
      stbi_image_free(pixels[i]);
      skip_log = false;
    }
  }
  if (skip_log) return;
  Log::verbose("[resources::cubemap_data] Cubemap data unloaded (path: {})", path);
}

cubemap_data::cubemap_data(cubemap_data&& c) noexcept :
  path(std::move(c.path)),
  width(c.width), height(c.height),
  format(c.format) {
  for (size_t i = 0; i < CUBEMAP_FACES; ++i) {
    pixels[i] = c.pixels[i];
    c.pixels[i] = nullptr;
  }
}

cubemap_data& cubemap_data::operator=(cubemap_data&& c) noexcept {
  Log::verbose("[resources::cubemap_data] Cubemap data overwritten (path: {} -> {})", path, c.path);

  path = std::move(c.path);
  width = c.width;
  height = c.height;
  format = c.format;

  for (size_t i = 0; i < CUBEMAP_FACES; ++i) {
    stbi_image_free(pixels[i]);
    pixels[i] = c.pixels[i];
    c.pixels[i] = nullptr;
  }

  return *this;
}

template<>
texture<texture2d_data>::texture(data_t data) :
  _path(std::move(data.path)),
  _texture(
    vec2sz{data.width, data.height},
    gl::texture::type::tex2d,
    data.format,
    &data.pixels
  ) {}

template<>
texture<cubemap_data>::texture(data_t data) :
  _path(std::move(data.path)),
  _texture(
    vec2sz{data.width, data.height},
    gl::texture::type::cubemap,
    data.format,
    data.pixels
  ) {}

} // namespace ntf::shogle::resources
