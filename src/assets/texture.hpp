#pragma once

#include "./assets.hpp"
#include "../render/render.hpp"

namespace ntf {

template<typename Texture>
struct basic_texture_data {
public:
  using texture_type = Texture;

  using pixels_type = texture_type::data_type;
  using dim_type = texture_type::dim_type;
  using params_type = texture_type::params_type;

public:
  pixels_type pixels{};
  dim_type dim{};
  tex_format format{};
  params_type params{};
};

template<typename Texture>
class stb_image_loader {
public:
  using resource_type = Texture;
  using data_type = basic_texture_data<Texture>;

private:
  using texture_type = Texture;

  using pixels_type = data_type::data_type;
  using dim_type = data_type::dim_type;
  using params_type = data_type::params_type;

public:
  bool resource_load(std::string_view path, params_type params);
  void resource_unload(bool overwrite);

  std::optional<resource_type> make_resource() const;

private:
  data_type _data;
};

template<typename T, typename Loader = stb_image_loader<T>>
using texture_data = resource_data<T, Loader>;

} // namespace ntf

#ifndef SHOGLE_ASSETS_TEXTURE_INL
#include "./texture.inl"
#endif
