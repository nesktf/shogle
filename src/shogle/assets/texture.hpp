#pragma once

#include "./common.hpp"
#include "../render/render.hpp"

namespace ntf {

template<typename Texture>
struct texture_data {
public:
  using texture_type = Texture;
  using data_type = texture_type::data_type;
  using dim_type = texture_type::dim_type;

  struct loader {
    texture_type operator()(texture_data data);
    texture_type operator()(std::string path, tex_filter filter, tex_wrap wrap);
  };

public:
  texture_data() = default;

  texture_data(std::string_view path_, tex_filter filter_, tex_wrap wrap_);

public:
  data_type pixels{};
  dim_type dim{};
  tex_format format{};
  tex_filter filter{};
  tex_wrap wrap{};

private:
  void unload();
  void invalidate();

public:
  NTF_DECLARE_MOVE_ONLY(texture_data);
};

} // namespace ntf

#ifndef SHOGLE_ASSETS_TEXTURE_INL
#include "./texture.inl"
#endif
