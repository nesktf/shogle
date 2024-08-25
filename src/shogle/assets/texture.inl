#define SHOGLE_ASSETS_TEXTURE_INL
#include <shogle/assets/texture.hpp>
#undef SHOGLE_ASSETS_TEXTURE_INL

#include <nlohmann/json.hpp>

#include <stb/stb_image.h>

namespace ntf {

template<typename Texture>
texture_data<Texture>::texture_data(std::string_view path_, tex_filter filter_, tex_wrap wrap_) :
  filter(filter_), wrap(wrap_) {
  constexpr size_t faces = texture_type::face_count;

  if constexpr (faces > 1) {
    using json = nlohmann::json;

    std::ifstream f{path_.data()};
    json data = json::parse(f);
    auto content = data["content"];
    assert(content.size() == faces && "Invalid json data");

    int w, h, ch;
    for (size_t i = 0; i < content.size(); ++i) {
      auto& curr = content[i];
      std::string path = file_dir(path_.data())+"/"+curr["path"].get<std::string>();
      pixels[i] = stbi_load(path.c_str(), &w, &h, &ch, 0);
      if (!pixels[i]) {
        throw ntf::error{"[ntf::texture_data] Error loading cubemap texture: {}, n: {}", path, i};
      }
    }

    dim = static_cast<size_t>(w);
    format = ntf::toenum(ch);
  } else {
    int w, h, ch;
    pixels = stbi_load(path_.data(), &w, &h, &ch, 0);
    if (!pixels) {
      throw ntf::error{"[ntf::texture_data] Error loading texture: {}", path_};
    }
    dim = ivec2{w, h};
    format = ntf::toenum(ch);
  }
}

template<typename Texture>
texture_data<Texture>::~texture_data() noexcept { unload(); }

template<typename Texture>
texture_data<Texture>::texture_data(texture_data&& d) noexcept :
  pixels(std::move(d.pixels)), dim(std::move(d.dim)), format(std::move(d.format)),
  filter(std::move(d.filter)), wrap(std::move(d.wrap)) { d.invalidate(); }

template<typename Texture>
auto texture_data<Texture>::operator=(texture_data&& d) noexcept -> texture_data& {
  unload();

  pixels = std::move(d.pixels);
  dim = std::move(d.dim);
  format = std::move(d.format);
  filter = std::move(d.filter);
  wrap = std::move(d.wrap);

  d.invalidate();

  return *this;
}

template<typename Texture>
void texture_data<Texture>::unload() {
  if constexpr (texture_type::face_count > 1) {
    for (auto& face : pixels) {
      if (face) {
        stbi_image_free(face);
      }
    }
  } else {
    if (pixels) {
      stbi_image_free(pixels);
    }
  }
}

template<typename Texture>
void texture_data<Texture>::invalidate() {
  if constexpr (texture_type::face_count > 1) {
    for (auto& face : pixels) {
      face = nullptr;
    }
  } else {
    pixels = nullptr;
  }
}

} // namespace ntf
