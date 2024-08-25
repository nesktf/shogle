#define SHOGLE_ASSETS_FONT_INL
#include <shogle/assets/font.hpp>
#undef SHOGLE_ASSETS_FONT_INL

namespace ntf {

template<typename Font>
font_data<Font>::font_data(std::string_view path) { 
  if (FT_Init_FreeType(&_ft_lib)) {
    throw ntf::error {"[ntf::font_data] Couldn't init freetype"};
  }
  if (FT_New_Face(_ft_lib, path.data(), 0, &_ft_face)) {
    throw ntf::error {"[ntf::font_data] Couldn't load font: {}", path};
  }

  FT_Set_Pixel_Sizes(_ft_face, 0, 48);

  for (uint8_t c = 0; c < 128; ++c) {
    if (FT_Load_Char(_ft_face, c, FT_LOAD_RENDER)) {
      log::warning("[ntf::font_data] Failed to load glyph {}", c);
      continue;
    }
    size_t sz = _ft_face->glyph->bitmap.width*_ft_face->glyph->bitmap.rows;
    _temp_glyphs.emplace_back(std::unique_ptr<uint8_t[]>(new uint8_t[sz]));
    uint8_t* data = _temp_glyphs.back().get();
    memcpy(data, _ft_face->glyph->bitmap.buffer, sz);
    glyphs.insert(std::make_pair(c, std::make_pair(data,
      font_glyph{
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

} // namespace ntf
