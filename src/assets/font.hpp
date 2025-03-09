#pragma once

#include "./types.hpp"

#include "../render/types.hpp"

namespace ntf {

struct font_glyph {
  uvec2 size;
  ivec2 bearing;
  ivec2 advance;
  uint32 x_offset;
};

template<typename T>
concept font_glyph_identifier = std::is_integral_v<T>;

template<font_glyph_identifier T>
struct bitmap_font_data {
public:
  using texel_array = unique_array<uint8>;
  using glyph_map = std::unordered_map<T, font_glyph>;

public:
  bitmap_font_data(texel_array texels_, glyph_map glyphs_) noexcept :
    texels{std::move(texels_)},
    glyphs{std::move(glyphs_)} {}

public:
  texel_array texels;
  glyph_map glyphs;
};

class ft2_bitmap_loader {
public:
  struct data_t {
    unique_array<uint8> texels;
    bitmap_font_data<uint8>::glyph_map glyphs;
  };

public:
  ft2_bitmap_loader() noexcept;
  ~ft2_bitmap_loader() noexcept;

public:
  asset_expected<data_t> parse(const std::string& path, std::string_view chars, uvec2 pixel_sz);

public:
  unique_array<uint8>&& texels(data_t& data) { return std::move(data.texels); }
  bitmap_font_data<uint8>::glyph_map&& glyphs(data_t& data) { return std::move(data.glyphs); }

private:
  void* _ft2_lib;
};

// struct basic_font_data {
//   glyph_map glyphs;
// };
//
// template<typename Font>
// class freetype2_loader {
// public:
//   using resource_type = Font;
//   using data_type = basic_font_data;
//
// private:
//   using font_type = Font;
//
// public:
//   // TODO: Pass custom allocator
//   bool resource_load(std::string_view path, uint8_t count = 128, uint pixel_size = 48);
//   void resource_unload(bool overwrite);
//
//   std::optional<resource_type> make_resource() const;
//
// public:
//   const data_type& data() const { return _data; }
//   data_type& data() { return _data; }
//
// private:
//   data_type _data;
// };
//
// template<typename T, typename Loader = freetype2_loader<T>>
// using font_data = resource_data<T, Loader>;
//
// template<typename Font>
// bool freetype2_loader<Font>::resource_load(std::string_view path, uint8_t count, uint pixel_size) {
//   FT_Library ft;
//   if (FT_Init_FreeType(&ft)) {
//     SHOGLE_LOG(error, "[ntf::freetype2_loader] Failed to initialize FreeType");
//     return false;
//   }
//
//   FT_Face face;
//   if (FT_New_Face(ft, path.data(), 0, &face)) {
//     FT_Done_FreeType(ft);
//     SHOGLE_LOG(error, "[ntf::freetype2_loader] Failed to load font \"{}\"", path);
//     return false;
//   }
//
//   if (FT_Set_Pixel_Sizes(face, 0, pixel_size)) {
//     FT_Done_Face(face);
//     FT_Done_FreeType(ft);
//     SHOGLE_LOG(error, "[ntf::freetype2_loader] Failed to set pixel size to {}px in font \"{}\"",
//                pixel_size, path);
//     return false;
//   }
//
//   glyph_map map;
//   for (uint8_t c = 0; c < count; ++c) {
//     if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
//       SHOGLE_LOG(warning, "[ntf::freetype2_loader] Failed to load glyph '{}' in font \"{}\"",
//                  static_cast<char>(c), path);
//       continue;
//     }
//     std::size_t sz = face->glyph->bitmap.width * face->glyph->bitmap.rows;
//     uint8_t* data = new uint8_t[sz];
//     std::memcpy(data, face->glyph->bitmap.buffer, sz);
//
//     map.insert(std::make_pair(c, std::make_pair(data, font_glyph {
//       .size = ivec2{
//         face->glyph->bitmap.width,
//         face->glyph->bitmap.rows
//       },
//       .bearing = ivec2{
//         face->glyph->bitmap_left,
//         face->glyph->bitmap_top
//       },
//       .advance = static_cast<unsigned long>(face->glyph->advance.x)
//     })));
//   }
//
//   FT_Done_Face(face);
//   FT_Done_FreeType(ft);
//
//   _data.glyphs = std::move(map);
//
//   return true;
// }
//
// template<typename Font>
// void freetype2_loader<Font>::resource_unload(bool overwrite) {
//   if (overwrite) {
//     SHOGLE_LOG(warning, "[ntf::freetype2_loader] Overwritting font data");
//   }
//
//   for (auto& [_, pair] : _data.glyphs) {
//     auto* data = pair.first;
//     delete[] data;
//   }
// }
//
// template<typename Font>
// auto freetype2_loader<Font>::make_resource() const -> std::optional<resource_type> {
//   auto font = font_type{_data.glyphs};
//   if (!font) {
//     return std::nullopt;
//   }
//   return {std::move(font)};
// }

} // namespace ntf
