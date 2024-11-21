#pragma once

#include "./common.hpp"
#include "../render/render.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <vector>

namespace ntf {

template<typename Font>
struct font_data {
public:
  using font_type = Font;

public:
  struct loader {
    font_type operator()(font_data data) {
      return font_type{std::move(data.glyphs)};
    }
    font_type operator()(std::string path) {
      return (*this)(font_data{path});
    }
  };

public:
  font_data(std::string_view path);

public:
  std::map<uint8_t, std::pair<uint8_t*, font_glyph>> glyphs;

private:
  std::vector<std::unique_ptr<uint8_t[]>> _temp_glyphs;
  FT_Face _ft_face;
  FT_Library _ft_lib;
};

} // namespace ntf

#ifndef SHOGLE_ASSETS_FONT_INL
#include "./font.inl"
#endif
