#define SHOGLE_ASSETS_TEXTURE_INL
#include "./texture.hpp"
#undef SHOGLE_ASSETS_TEXTURE_INL

#include <nlohmann/json.hpp>

#include <stb/stb_image.h>

namespace ntf {

template<typename Texture>
auto stb_image_loader<Texture>::_load_data(std::string_view path, params_type params)
                                                                    -> std::optional<data_type> {
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

  data_type out;
  constexpr auto faces = texture_type::face_count;
  if constexpr (faces > 1u) {
    using json = nlohmann::json;

    std::ifstream f{path.data()};
    json data = json::parse(f);
    auto content = data["content"];
    if (content.size() != faces) {
      SHOGLE_LOG(error, "[ntf::stb_image_loader] Invalid cubemap data in file \"{}\"", path);
      return std::nullopt;
    }

    int w, h, ch;
    uint8_t* pixels[faces];
    for (std::size_t i = 0; i < content.size(); ++i) {
      auto& curr = content[i];
      std::string face_path = file_dir(path.data())+"/"+curr["path"].get<std::string>();
      pixels[i] = stbi_load(face_path.c_str(), &w, &h, &ch, 0);
      if (!pixels[i]) {
        for (int j = static_cast<int>(i)-1; j >= 0; --j) {
          stbi_image_free(pixels[j]);
        }
        SHOGLE_LOG(error, "[ntf::stb_image_loader] Failed to load cubemap texture \"{}\" (n: {})",
                   path, i);
        return std::nullopt;
      }
    }

    for (std::size_t i = 0; i < faces; ++i) {
      out.pixels[i] = pixels[i];
    }
    out.dim = static_cast<std::size_t>(w);
    out.format = toenum(ch);
  } else {
    int w, h, ch;
    uint8_t* pixels = stbi_load(path.data(), &w, &h, &ch, 0);
    if (!pixels) {
      SHOGLE_LOG(error, "[ntf::stb_image_loader] Failed to load texture \"{}\"", path);
      return std::nullopt;
    }
    out.pixels = pixels;
    out.dim = ivec2{w, h};
    out.format = toenum(ch);
  }
  out.params = params;

  return {std::move(out)};
}

template<typename Texture>
void stb_image_loader<Texture>::_unload_data(data_type& data) {
  constexpr auto faces = texture_type::face_count;
  if constexpr (faces > 1u) {
    for (auto& face : data.pixels) {
      if (face) {
        stbi_image_free(face);
      }
    }
  } else {
    if (data.pixels) {
      stbi_image_free(data.pixels);
    }
  }
}


template<typename Texture, typename Loader>
template<typename... Args>
texture_data<Texture, Loader>::texture_data(Args&&... args) {
  _load(std::forward<Args>(args)...);
}

template<typename Texture, typename Loader>
template<typename... Args>
auto texture_data<Texture, Loader>::load(Args&&... args) & -> texture_data& {
  _load(std::forward<Args>(args)...);
  return *this;
}

template<typename Texture, typename Loader>
template<typename... Args>
auto texture_data<Texture, Loader>::load(Args&&... args) && -> texture_data&& {
  _load(std::forward<Args>(args)...);
  return std::move(*this);
}

template<typename Texture, typename Loader>
auto texture_data<Texture, Loader>::load_resource() -> std::optional<texture_type> {
  if (!has_data()) {
    return std::nullopt;
  }

  auto texture = texture_type{_data.pixels, _data.dim, _data.format, _data.params};
  if (!texture) {
    return std::nullopt;
  }

  return {std::move(texture)};
}

template<typename Texture, typename Loader>
void texture_data<Texture, Loader>::unload() {
  if (!has_data()) {
    return;
  }

  this->_unload_data(_data);

  _reset();
}


template<typename Texture, typename Loader>
template<typename... Args>
void texture_data<Texture, Loader>::_load(Args&&... args) {
  auto data = this->_load_data(std::forward<Args>(args)...);
  if (!data) {
    return;
  }
  _data = std::move(data.value());
  _has_data = true;
}


template<typename Texture, typename Loader>
void texture_data<Texture, Loader>::_reset() {
  // _data.pixels = {};
  // _data.dim = {};
  // _data.format = {};
  // _data.params = {};
  _has_data = false;
}


template<typename Texture, typename Loader>
texture_data<Texture, Loader>::~texture_data() noexcept { unload(); }

template<typename Texture, typename Loader>
texture_data<Texture, Loader>::texture_data(texture_data&& d) noexcept :
  _data(std::move(d._data)), _has_data(d._has_data) { d._reset(); }

template<typename Texture, typename Loader>
auto texture_data<Texture, Loader>::operator=(texture_data&& d) noexcept -> texture_data& {
  unload();

  _has_data = std::move(d._has_data);
  if (_has_data) {
    _data = std::move(d._data);
  }

  d._reset();

  return *this;
}

} // namespace ntf
