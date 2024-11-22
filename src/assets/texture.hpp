#pragma once

#include "./common.hpp"
#include "../render/render.hpp"

#include <optional>

namespace ntf {

namespace impl {

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

} // namespace impl

template<typename Texture>
class stb_image_loader {
public:
  using texture_type = Texture;

private:
  using data_type = impl::basic_texture_data<Texture>;

  using pixels_type = data_type::data_type;
  using dim_type = data_type::dim_type;
  using params_type = data_type::params_type;

public:
  std::optional<data_type> _load_data(std::string_view path, params_type params);
  void _unload_data(data_type& data);
};

template<typename Texture, typename Loader = stb_image_loader<Texture>>
class texture_data : private Loader {
public:
  using texture_type = Texture;
  using loader_type = Loader;

  using data_type = impl::basic_texture_data<texture_type>;

public:
  texture_data() = default;

  template<typename... Args>
  texture_data(Args&&... args);

public:
  template<typename... Args>
  texture_data& load(Args&&... args) &;

  template<typename... Args>
  texture_data&& load(Args&&... args) &&;

  void unload();

  std::optional<texture_type> load_resource();

public:
  auto& pixels() { return _data.pixels; }
  auto dim() const { return _data.dim; } 
  auto params() const { return _data.params; }

  bool has_data() const { return _has_data; }
  explicit operator bool() const { return has_data(); }

private:
  template<typename... Args>
  void _load(Args&&... args);

  void _reset();

private:
  impl::basic_texture_data<Texture> _data;
  bool _has_data{false};

public:
  NTF_DECLARE_MOVE_ONLY(texture_data);
};

} // namespace ntf

#ifndef SHOGLE_ASSETS_TEXTURE_INL
#include "./texture.inl"
#endif
