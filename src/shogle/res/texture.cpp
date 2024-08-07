#include <shogle/res/texture.hpp>
#include <shogle/res/util.hpp>

#include <shogle/core/error.hpp>

#include <nlohmann/json.hpp>

#include <stb/stb_image.h>

#include <fstream>

using json = nlohmann::json;

namespace ntf::shogle {

static tex_format __toenum(int channels) {
  switch (channels) {
    case 1:
      return tex_format::mono;
    case 4:
      return tex_format::rgba;
    default:
      return tex_format::rgb;
  }
}

texture2d_data::texture2d_data(std::string_view path_) {
  int w, h, ch;
  pixels = stbi_load(path_.data(), &w, &h, &ch, 0);
  if (!pixels) {
    throw ntf::error{"[shogle::texture_data] Error loading texture: {}", path_};
  }

  width = static_cast<size_t>(w);
  height = static_cast<size_t>(h);
  format = __toenum(ch);
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
  std::ifstream f{path_.data()};
  json data = json::parse(f);

  auto content = data["content"];
  assert(content.size() == CUBEMAP_FACES && "Invalid json data");

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
  format = __toenum(ch);
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

} // namespace ntf::shogle
