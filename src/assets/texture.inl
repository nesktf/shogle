#define SHOGLE_ASSETS_TEXTURE_INL
#include "./texture.hpp"
#undef SHOGLE_ASSETS_TEXTURE_INL

#include <nlohmann/json.hpp>

#include <stb/stb_image.h>

namespace ntf {

template<typename Texture>
bool stb_image_loader<Texture>::resource_load(std::string_view path, params_type params) {
  auto toenum = [](int channels) -> tex_format {
    switch (channels) {
      case 1:
        return tex_format::mono;
      case 4:
        return tex_format::rgba;
      default:
        return tex_format::rgb;
    }
  };

  constexpr auto faces = texture_type::face_count;
  if constexpr (faces > 1u) {
    using json = nlohmann::json;

    std::ifstream f{path.data()};
    json data = json::parse(f);
    auto content = data["content"];
    if (content.size() != faces) {
      SHOGLE_LOG(error, "[ntf::stb_image_loader] Invalid cubemap data in file \"{}\"", path);
      return false;
    }

    int w, h, ch;
    uint8_t* pixels[faces];
    for (std::size_t i = 0; i < content.size(); ++i) {
      auto& curr = content[i];

      auto dir = file_dir(path.data());
      if (!dir) {
        SHOGLE_LOG(error, "[ntf::stb_image_loader] Invalid texture path \"{}\"", path);
        return false;
      }
      std::string face_path = dir.value()+"/"+curr["path"].get<std::string>();

      pixels[i] = stbi_load(face_path.c_str(), &w, &h, &ch, 0);
      if (!pixels[i]) {
        for (int j = static_cast<int>(i)-1; j >= 0; --j) {
          stbi_image_free(pixels[j]);
        }
        SHOGLE_LOG(error, "[ntf::stb_image_loader] Failed to load cubemap texture \"{}\" (n: {})",
                   path, i);
        return false;
      }
    }

    for (std::size_t i = 0; i < faces; ++i) {
      _data.pixels[i] = pixels[i];
    }
    _data.dim = static_cast<std::size_t>(w);
    _data.format = toenum(ch);
  } else {
    int w, h, ch;
    uint8_t* pixels = stbi_load(path.data(), &w, &h, &ch, 0);
    if (!pixels) {
      SHOGLE_LOG(error, "[ntf::stb_image_loader] Failed to load texture \"{}\"", path);
      return false;
    }
    _data.pixels = pixels;
    _data.dim = ivec2{w, h};
    _data.format = toenum(ch);
  }
  _data.params = params;

  return true;
}

template<typename Texture>
void stb_image_loader<Texture>::resource_unload(bool overwrite) {
  if (overwrite) {
    SHOGLE_LOG(warning, "[ntf::stb_image_loader] Overwritting texture");
  }

  constexpr auto faces = texture_type::face_count;
  if constexpr (faces > 1u) {
    for (auto& face : _data.pixels) {
      if (face) {
        stbi_image_free(face);
      }
    }
  } else {
    if (_data.pixels) {
      stbi_image_free(_data.pixels);
    }
  }
}

template<typename Texture>
auto stb_image_loader<Texture>::make_resource() const -> std::optional<resource_type> {
  auto tex = texture_type{_data.pixels, _data.dim, _data.format, _data.params};
  if (!tex) {
    return std::nullopt;
  }
  return {std::move(tex)};
}

} // namespace ntf
