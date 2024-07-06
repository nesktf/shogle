#include <shogle/res/font.hpp>

#include <shogle/core/error.hpp>

#include <shogle/core/log.hpp>

namespace ntf::shogle {

font_data::font_data(std::string_view path) { 
  if (FT_Init_FreeType(&_ft_lib)) {
    throw ntf::error {"[shogle::font_data] Couldn't init freetype"};
  }
  if (FT_New_Face(_ft_lib, path.data(), 0, &_ft_face)) {
    throw ntf::error {"[shogle::font_data] Couldn't load font: {}", path};
  }

  FT_Set_Pixel_Sizes(_ft_face, 0, 48);

  for (uint8_t c = 0; c < 128; ++c) {
    if (FT_Load_Char(_ft_face, c, FT_LOAD_RENDER)) {
      log::warning("[shogle::font_data] Failed to load glyph {}", c);
      continue;
    }
    size_t sz = _ft_face->glyph->bitmap.width*_ft_face->glyph->bitmap.rows;
    uint8_t* data = new uint8_t[sz];
    memcpy(data, _ft_face->glyph->bitmap.buffer, sz);
    chars.insert(std::make_pair(c, std::make_pair(data,
      font::character{
        .size = ivec2{
          _ft_face->glyph->bitmap.width,
          _ft_face->glyph->bitmap.rows
        },
        .bearing = ivec2{
          _ft_face->glyph->bitmap_left,
          _ft_face->glyph->bitmap_top
        },
        .advance = static_cast<unsigned long>(_ft_face->glyph->advance.x)
      }
    )));
  }
  FT_Done_Face(_ft_face);
  FT_Done_FreeType(_ft_lib);
}

font_data::~font_data() {

}

font load_font(std::string_view path) {
  return load_font(font_data{path});
}

font load_font(font_data data) {
  return font{std::move(data.chars)};
}

} // namespace ntf::shogle
