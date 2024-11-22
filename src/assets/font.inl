#define SHOGLE_ASSETS_FONT_INL
#include "./font.hpp"
#undef SHOGLE_ASSETS_FONT_INL

namespace ntf {

template<typename Font>
bool freetype2_loader<Font>::resource_load(std::string_view path, uint8_t count, uint pixel_size) {
  FT_Library ft;
  if (FT_Init_FreeType(&ft)) {
    SHOGLE_LOG(error, "[ntf::freetype2_loader] Failed to initialize FreeType");
    return false;
  }

  FT_Face face;
  if (FT_New_Face(ft, path.data(), 0, &face)) {
    FT_Done_FreeType(ft);
    SHOGLE_LOG(error, "[ntf::freetype2_loader] Failed to load font \"{}\"", path);
    return false;
  }

  if (FT_Set_Pixel_Sizes(face, 0, pixel_size)) {
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    SHOGLE_LOG(error, "[ntf::freetype2_loader] Failed to set pixel size to {}px in font \"{}\"",
               pixel_size, path);
    return false;
  }

  glyph_map map;
  for (uint8_t c = 0; c < count; ++c) {
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      SHOGLE_LOG(warning, "[ntf::freetype2_loader] Failed to load glyph '{}' in font \"{}\"",
                 static_cast<char>(c), path);
      continue;
    }
    std::size_t sz = face->glyph->bitmap.width * face->glyph->bitmap.rows;
    uint8_t* data = new uint8_t[sz];
    std::memcpy(data, face->glyph->bitmap.buffer, sz);

    map.insert(std::make_pair(c, std::make_pair(data, font_glyph {
      .size = ivec2{
        face->glyph->bitmap.width,
        face->glyph->bitmap.rows
      },
      .bearing = ivec2{
        face->glyph->bitmap_left,
        face->glyph->bitmap_top
      },
      .advance = static_cast<unsigned long>(face->glyph->advance.x)
    })));
  }

  FT_Done_Face(face);
  FT_Done_FreeType(ft);

  _data.glyphs = std::move(map);

  return true;
}

template<typename Font>
void freetype2_loader<Font>::resource_unload(bool overwrite) {
  if (overwrite) {
    SHOGLE_LOG(warning, "[ntf::freetype2_loader] Overwritting font data");
  }

  for (auto& [_, pair] : _data.glyphs) {
    auto* data = pair.first;
    delete[] data;
  }
}

template<typename Font>
auto freetype2_loader<Font>::make_resource() const -> std::optional<resource_type> {
  auto font = font_type{_data.glyphs};
  if (!font) {
    return std::nullopt;
  }
  return {std::move(font)};
}

} // namespace ntf
