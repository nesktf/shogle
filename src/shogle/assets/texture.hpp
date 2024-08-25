#pragma once

#include <shogle/render/common.hpp>

#include <shogle/assets/common.hpp>

namespace ntf {

template<typename Texture>
struct texture_data {
public:
  using texture_type = Texture;
  using data_type = texture_type::data_type;
  using dim_type = texture_type::dim_type;

  struct loader {
    texture_type operator()(texture_data data) {
      typename texture_type::loader tex_loader;
      return tex_loader(data.pixels, data.dim, data.format, data.filter, data.wrap);
    }
    texture_type operator()(std::string_view path, tex_filter filter, tex_wrap wrap) {
      return (*this)(texture_data{path, filter, wrap});
    }
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
#include <shogle/assets/texture.inl>
#endif
