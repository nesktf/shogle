#include "./font.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#define RET_ERR(msg, ...) \
  SHOGLE_LOG(error, "[ntf::ft2_bitmap_loader] " msg __VA_OPT__(,) __VA_ARGS__); \
  return unexpected{asset_error::format({msg}__VA_OPT__(,) __VA_ARGS__)};

#define RET_ERR_IF(cond, ...) \
  if (cond) { RET_ERR(__VA_ARGS__); }

namespace ntf {

namespace {

// Assume FT_Face is just a pointer handle
static_assert(std::is_pointer_v<FT_Face>);
using face_clean_t =
  std::unique_ptr<std::remove_pointer_t<FT_Face>, decltype([](FT_Face f){ FT_Done_Face(f); })>;

} // namespace

ft2_bitmap_loader::ft2_bitmap_loader() noexcept {
  FT_Library lib;
  if (FT_Init_FreeType(&lib)) {
    SHOGLE_LOG(error, "[ntf::ft2_bitmap_loader] Failed to initialize FreeType handle");
    _ft2_lib = nullptr;
    return;
  }
  // Assume FT_Library is just a pointer handle
  static_assert(std::is_pointer_v<FT_Library>);
  _ft2_lib = static_cast<void*>(lib);
}

ft2_bitmap_loader::~ft2_bitmap_loader() noexcept {
  if (!_ft2_lib) {
    return;
  }
  FT_Done_FreeType(static_cast<FT_Library>(_ft2_lib));
}

auto ft2_bitmap_loader::_load_face(const std::string& path,
                                   const uvec2& glyph_size) -> asset_expected<face_t> {
  RET_ERR_IF(!_ft2_lib, "Failed to initialize FreeType");
  FT_Library ft = static_cast<FT_Library>(_ft2_lib);

  FT_Face face;
  RET_ERR_IF(FT_New_Face(ft, path.c_str(), 0, &face), "Failed to load font");

  RET_ERR_IF(FT_Set_Pixel_Sizes(face, glyph_size.x, glyph_size.y),
             "Failed to set pixel size to {} {}",
             glyph_size.x, glyph_size.y);

  static_assert(std::is_pointer_v<FT_Face>); // Assume FT_Face is just a pointer handle
  return static_cast<face_t>(face);
}

void ft2_bitmap_loader::_unload_face(face_t face) {
  FT_Done_Face(static_cast<FT_Face>(face));
}

auto ft2_bitmap_loader::_load_glyph(face_t face, uint64 code) -> optional<ft_glyph_data> {
  FT_Face ft_face = static_cast<FT_Face>(face);
  if (FT_Load_Char(ft_face, static_cast<FT_ULong>(code), FT_LOAD_BITMAP_METRICS_ONLY)) {
    return nullopt;
  }

  const auto* g = ft_face->glyph;
  return ft_glyph_data{
    .size = {static_cast<uint32>(g->bitmap.width), static_cast<uint32>(g->bitmap.rows)},
    .bearing {static_cast<int32>(g->bitmap_left), static_cast<int32>(g->bitmap_top)},
    .advance = {static_cast<int32>(g->advance.x>>6), static_cast<int32>(g->advance.y>>6)},
  };
}

auto ft2_bitmap_loader::load(font_load_t<char, uint8>,
                             const std::string& path,
                             font_charset_view<char> charset,
                             uvec2 glyph_size) -> asset_expected<data_t<char, uint8>> {
  face_clean_t face_clean{face};

  RET_ERR_IF(FT_Set_Pixel_Sizes(face, glyph_size.x, glyph_size.y),
             "Failed to set pixel size to {} {}",
             glyph_size.x, glyph_size.y);

  const auto alignment = r_image_alignment::bytes1;
  const auto tex_format = tex_depth_traits<uint8>::parse_channels(1u | TEX_DEPTH_NORMALIZE_BIT);
  NTF_ASSERT(tex_format.has_value());

  // Extract glyph metadata
  std::unordered_map<char, glyph_data<char>> glyph_map;
  size_t image_size = 0;
  for (const auto ch : charset) {
    if (FT_Load_Char(face, static_cast<FT_ULong>(ch), FT_LOAD_BITMAP_METRICS_ONLY)) {
      SHOGLE_LOG(warning, "[ntf::ft2_bitmap_loader] Failed to load character '{}'", ch);
      continue;
    }
    const auto* g = face->glyph;
    const auto glyph_w = static_cast<uint32>(g->bitmap.width);
    const auto glyph_h = static_cast<uint32>(g->bitmap.rows);

    const auto [it, emplaced] = glyph_map.try_emplace(static_cast<uint8>(ch), glyph_data<char>{
      .id = ch,
      .offset = image_size,
      .size = {glyph_w, glyph_h},
      .bearing {static_cast<int32>(g->bitmap_left), static_cast<int32>(g->bitmap_top)},
      .advance = {static_cast<int32>(g->advance.x>>6), static_cast<int32>(g->advance.y>>6)},
    });
    if (!emplaced) {
      SHOGLE_LOG(warning, "[ntf::ft2_bitmap_loader] Ignoring duplicate character '{}'", ch);
      continue;
    }
    image_size += static_cast<size_t>(glyph_w*glyph_h);
  }

  auto texels = unique_array<uint8>::from_allocator(::ntf::uninitialized, image_size);
  RET_ERR_IF(!texels.has_data(), "Failed to allocate texels");
  auto glyphs = unique_array<glyph_data<char>>::from_allocator(::ntf::uninitialized,
                                                               glyph_map.size());
  RET_ERR_IF(!texels.has_data(), "Failed to allocate glyphs");

  // Copy bitmap texels
  size_t idx = 0;
  for (const auto& [ch, glyph] : glyph_map) {
    if (FT_Load_Char(face, static_cast<FT_ULong>(ch), FT_LOAD_RENDER)) {
      continue;
    }

    const auto* g = face->glyph;
    std::memcpy(texels.get()+glyph.offset, g->bitmap.buffer, g->bitmap.width*g->bitmap.rows);
    std::construct_at(glyphs.get()+idx, std::move(glyph));
    ++idx;
  }
  SHOGLE_LOG(debug, "[ntf::ft2_bitmap_loader] Loaded {} glyphs, generated {} texels",
             glyphs.size(), texels.size());

  return data_t<char, uint8>{std::move(texels), std::move(glyphs), *tex_format, alignment};
}

} // namespace ntf
