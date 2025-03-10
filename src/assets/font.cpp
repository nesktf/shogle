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

auto ft2_bitmap_loader::parse(const std::string& path,
                              std::string_view chars,
                              uvec2 pixel_sz) -> asset_expected<data_t> {
  RET_ERR_IF(!_ft2_lib, "Failed to initialize FreeType");
  FT_Library ft = static_cast<FT_Library>(_ft2_lib);

  FT_Face face;
  RET_ERR_IF(FT_New_Face(ft, path.c_str(), 0, &face), "Failed to load font");
  face_clean_t face_clean{face};

  RET_ERR_IF(FT_Set_Pixel_Sizes(face, pixel_sz.x, pixel_sz.y), "Failed to set pixel size to {} {}",
             pixel_sz.x, pixel_sz.y);

  // Extract glyph metadata
  uint32 tex_w = 0;
  uint32 tex_h = 0;
  bitmap_font_data<uint8>::glyph_map glyphs;
  for (const auto ch : chars) {
    if (FT_Load_Char(face, static_cast<FT_ULong>(ch), FT_LOAD_BITMAP_METRICS_ONLY)) {
      SHOGLE_LOG(warning, "[ntf::ft2_bitmap_loader] Failed to load character '{}'", ch);
      continue;
    }
    const auto* g = face->glyph;
    const auto glyph_w = static_cast<uint32>(g->bitmap.width);
    const auto glyph_h = static_cast<uint32>(g->bitmap.rows);

    const auto [it, emplaced] = glyphs.try_emplace(static_cast<uint8>(ch), font_glyph{
      .size = {glyph_w, glyph_h},
      .bearing {static_cast<int32>(g->bitmap_left), static_cast<int32>(g->bitmap_top)},
      .advance = {static_cast<int32>(g->advance.x>>6), static_cast<int32>(g->advance.y>>6)},
      .x_offset = tex_w,
    });
    if (!emplaced) {
      SHOGLE_LOG(warning, "[ntf::ft2_bitmap_loader] Ignoring duplicate character '{}'", ch);
      continue;
    }
    // SHOGLE_LOG(debug, "CHAR: {}", ch);
    // SHOGLE_LOG(debug, "- sz:   {} {}", it->second.size.x, it->second.size.y);
    // SHOGLE_LOG(debug, "- bear: {} {}", it->second.bearing.x, it->second.bearing.y);
    // SHOGLE_LOG(debug, "- adv:  {} {}", it->second.advance.x, it->second.advance.y);
    // SHOGLE_LOG(debug, "- off:  {}", it->second.x_offset);

    tex_w += glyph_w;
    tex_h = std::max(tex_h, glyph_h);
  };

  const size_t texels_sz = static_cast<size_t>(tex_w*tex_h)*sizeof(uint8);
  auto texels = unique_array<uint8>::from_allocator(::ntf::uninitialized, texels_sz);
  RET_ERR_IF(!texels, "Failed to allocate texels");

  // Copy bitmap texels
  for (const auto& [ch, glyph] : glyphs) {
    if (FT_Load_Char(face, static_cast<FT_ULong>(ch), FT_LOAD_RENDER)) {
      continue;
    }

    const auto* g = face->glyph;
    const size_t w = g->bitmap.width;
    for (size_t row = 0; row < g->bitmap.rows; ++row) {
      uint8* buff_pos = g->bitmap.buffer + row*w;
      uint8* tex_pos = texels.get() + (row*tex_w + glyph.x_offset);
      std::memcpy(tex_pos, buff_pos, w);
    }
  }
  SHOGLE_LOG(debug, "[ntf::ft2_bitmap_loader] Loaded {} glyphs, generated {}x{} bitmap",
             glyphs.size(), tex_w, tex_h);

  return data_t{std::move(texels), std::move(glyphs)};
}

} // namespace ntf
