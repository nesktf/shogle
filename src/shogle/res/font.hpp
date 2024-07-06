#pragma once

#include <shogle/render/font.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace ntf::shogle {

struct font_data {
  font_data(std::string_view path);
  ~font_data();

  std::map<uint8_t, std::pair<uint8_t*, font::character>> chars;

  FT_Face _ft_face;
  FT_Library _ft_lib;

  // font_data(font_data&&) noexcept;
  font_data(const font_data&) = delete;
  // font_data& operator=(font_data&&) noexcept;
  font_data& operator=(const font_data&) = delete;
};

font load_font(std::string_view path);
font load_font(font_data data);

} // namespace ntf::shogle
