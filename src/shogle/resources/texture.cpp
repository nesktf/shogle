#include <shogle/resources/texture.hpp>

#include <shogle/resources/util.hpp>

#include <shogle/core/error.hpp>
#include <shogle/core/log.hpp>

#include <stb/stb_image.h>

#include <nlohmann/json.hpp>

#include <fstream>

namespace ntf::shogle::resources {

static gl::texture::format to_enum(int channels) {
  switch (channels) {
    case 1:
      return gl::texture::format::mono;
    case 4:
      return gl::texture::format::rgba;
    default:
      return gl::texture::format::rgb;
  }
}

texture2d_data::texture2d_data(std::string _path) :
  path(std::move(_path)) {
  pixels = stbi_load(path.c_str(), &width, &height, &channels, 0);
  if (!pixels) {
    throw ntf::error{"[resources::texture_data] Error loading texture: {}", path};
  }
  format = to_enum(channels);
  log::verbose("[resources::texture_data] Texture data loaded (path: {})", path);
}

texture2d_data::~texture2d_data() {
  if (pixels) {
    stbi_image_free(pixels);
    log::verbose("[resources::texture_data] Texture data unloaded (path: {})", path);
  }
}

texture2d_data::texture2d_data(texture2d_data&& t) noexcept :
  path(std::move(t.path)),
  width(std::move(t.width)), height(std::move(t.height)), channels(std::move(t.channels)),
  format(std::move(t.format)),
  pixels(std::move(t.pixels)) { t.pixels = nullptr; }

texture2d_data& texture2d_data::operator=(texture2d_data&& t) noexcept {
  if (pixels) {
    stbi_image_free(pixels);
    log::verbose("[resources::texture_data] Texture data overwritten (path: {} -> {})", path, t.path);
  }

  path = std::move(t.path);
  width = std::move(t.width);
  height = std::move(t.height);
  channels = std::move(t.channels);
  format = std::move(t.format);
  pixels = std::move(t.pixels);

  t.pixels = nullptr;

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

  format = to_enum(channels);
  log::verbose("[resources::cubemap_data] Cubemap data loaded (path: {})", path);
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
  log::verbose("[resources::cubemap_data] Cubemap data unloaded (path: {})", path);
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
  log::verbose("[resources::cubemap_data] Cubemap data overwritten (path: {} -> {})", path, c.path);

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

} // namespace ntf::shogle::resources
