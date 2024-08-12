#pragma once

#include <shogle/core/error.hpp>

#include <shogle/render/texture.hpp>

#include <shogle/res/util.hpp>

#include <nlohmann/json.hpp>

#include <stb/stb_image.h>

namespace ntf {

struct spritesheet_data;

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


template<size_t faces>
struct texture_data {
public:
  using data_type = typename tex_helper<faces>::data_type;
  using dim_type = typename tex_helper<faces>::dim_type;
  using texture_type = texture<faces>;

  struct loader {
    texture_type operator()(texture_data data) {
      return impl::load_texture<faces>(data.pixels, data.dim, data.format, data.filter, data.wrap);
    }
  };

public:
  texture_data(std::string_view path_, tex_filter filter_, tex_wrap wrap_);

public:
  data_type pixels{};
  dim_type dim{};
  tex_format format{};
  tex_filter filter{};
  tex_wrap wrap{};

private:
  void unload_pixels();
  void invalidate_pixels();
  texture_data() = default;

private:
  friend struct ntf::spritesheet_data;

public:
  ~texture_data();
  texture_data(texture_data&&) noexcept;
  texture_data(const texture_data&) = delete;
  texture_data& operator=(texture_data&&) noexcept;
  texture_data& operator=(const texture_data&) = delete;
};


template<size_t faces>
texture_data<faces>::texture_data(std::string_view path_, tex_filter filter_, tex_wrap wrap_) :
  filter(filter_), wrap(wrap_) {
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
  format = impl::toenum(ch);
}

template<>
inline texture_data<1u>::texture_data(std::string_view path_, tex_filter filter_, tex_wrap wrap_) :
  filter(filter_), wrap(wrap_) {
  int w, h, ch;
  pixels = stbi_load(path_.data(), &w, &h, &ch, 0);
  if (!pixels) {
    throw ntf::error{"[ntf::texture_data] Error loading texture: {}", path_};
  }
  dim = ivec2{w, h};
  format = impl::toenum(ch);
}

template<size_t faces>
void texture_data<faces>::unload_pixels() {
  for (auto& face : pixels) {
    if (face) {
      stbi_image_free(face);
    }
  }
}

template<>
inline void texture_data<1u>::unload_pixels() {
  if (pixels) {
    stbi_image_free(pixels);
  }
}

template<size_t faces>
void texture_data<faces>::invalidate_pixels() {
  for (auto& face : pixels) {
    face = nullptr;
  }
}

template<>
inline void texture_data<1u>::invalidate_pixels() {
  pixels = nullptr;
}

template<size_t faces>
texture_data<faces>::~texture_data() { unload_pixels(); }

template<size_t faces>
texture_data<faces>::texture_data(texture_data&& d) noexcept :
  pixels(std::move(d.pixels)), dim(std::move(d.dim)), format(std::move(d.format)),
  filter(std::move(d.filter)), wrap(std::move(d.wrap)) { d.invalidate_pixels(); }

template<size_t faces>
auto texture_data<faces>::operator=(texture_data&& d) noexcept -> texture_data& {
  unload_pixels();

  pixels = std::move(d.pixels);
  dim = std::move(d.dim);
  format = std::move(d.format);
  filter = std::move(d.filter);
  wrap = std::move(d.wrap);

  d.invalidate_pixels();

  return *this;
}

} // namespace impl


using texture2d_data = impl::texture_data<1u>;
using cubemap_data = impl::texture_data<SHOGLE_CUBEMAP_FACES>;


inline texture2d load_texture(std::string_view path, tex_filter filter, tex_wrap wrap) {
  texture2d_data::loader loader;
  return loader(texture2d_data{path, filter, wrap});
}

inline cubemap load_cubemap(std::string_view path, tex_filter filter, tex_wrap wrap) {
  cubemap_data::loader loader;
  return loader(cubemap_data{path, filter, wrap});
}

} // namespace ntf
