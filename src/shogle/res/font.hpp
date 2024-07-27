#pragma once

#include <shogle/render/font.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace ntf::shogle {

struct font_data {
public:
  font_data(std::string_view path);

public:
  std::map<uint8_t, std::pair<uint8_t*, font_glyph>> glyphs;

public:
  ~font_data() = default;
  font_data(font_data&&) = default;
  font_data(const font_data&) = delete;
  font_data& operator=(font_data&&) = default;
  font_data& operator=(const font_data&) = delete;

private:
  std::vector<uptr<uint8_t[]>> _temp_glyphs;
  FT_Face _ft_face;
  FT_Library _ft_lib;
};

inline font load_font(std::string_view path) {
  auto data = font_data{path};
  return font{std::move(data.glyphs)};
}

} // namespace ntf::shogle
