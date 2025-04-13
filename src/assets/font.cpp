#include "./font.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#define RET_ERR(msg, ...) \
  SHOGLE_LOG(error, "[ntf::ft2_bitmap_loader] " msg __VA_OPT__(,) __VA_ARGS__); \
  return unexpected{asset_error::format({msg}__VA_OPT__(,) __VA_ARGS__)};

#define RET_ERR_IF(cond, ...) \
  if (cond) { RET_ERR(__VA_ARGS__); }

namespace ntf {

void ft2_font_loader::_unload_lib(void* lib) {
  static_assert(std::is_pointer_v<FT_Library>); // Assume FT_Library is just a pointer handle
  FT_Done_FreeType(static_cast<FT_Library>(lib));
}

void ft2_font_loader::_unload_face(void* face) {
  static_assert(std::is_pointer_v<FT_Face>); // Assume FT_Face is just a pointer handle
  FT_Done_Face(static_cast<FT_Face>(face));
}

ft2_font_loader::ft2_font_loader() noexcept :
  _ft2_lib{nullptr, lib_del_t{}}
{
  FT_Library lib;
  if (FT_Init_FreeType(&lib)) {
    SHOGLE_LOG(error, "[ntf::ft2_bitmap_loader] Failed to initialize FreeType handle");
    return;
  }
  _ft2_lib.reset(static_cast<void*>(lib));
}

auto ft2_font_loader::_load_face(span_view<uint8> file_data,
                                 const extent2d& glyph_size) -> asset_expected<face_t> {
  RET_ERR_IF(!_ft2_lib, "Failed to initialize FreeType");
  FT_Library ft = static_cast<FT_Library>(_ft2_lib.get());

  FT_Face face;
  RET_ERR_IF(FT_New_Memory_Face(ft, file_data.data(), file_data.size(), 0, &face),
             "Failed to load font");

  // TODO: Handle fixed size fonts
  RET_ERR_IF(!FT_IS_SCALABLE(face),
             "Unsupported font");

  RET_ERR_IF(FT_Set_Pixel_Sizes(face, glyph_size.x, glyph_size.y),
             "Failed to set pixel size to {} {}",
             glyph_size.x, glyph_size.y);

  RET_ERR_IF(FT_Select_Charmap(face, FT_ENCODING_UNICODE),
             "Failed to set font charmap encoding");

  return face_t{face, face_del_t{}};
}

auto ft2_font_loader::_get_code_index(const face_t& face, uint64 code) -> uint32 {
  FT_Face ft_face = static_cast<FT_Face>(face.get());
  return static_cast<uint32>(FT_Get_Char_Index(ft_face, static_cast<FT_ULong>(code)));
}

auto ft2_font_loader::_load_metrics(const face_t& face, uint32 idx,
                                    ft_mode load_mode) -> optional<ft_glyph_data> {
  FT_Int32 load_flags;
  switch (load_mode) {
    case ft_mode::sdf: [[fallthrough]]; // SDF doesn't have a loading hint
    case ft_mode::normal: {
      load_flags = FT_LOAD_TARGET_NORMAL;
      break;
    }
  }
  FT_Face ft_face = static_cast<FT_Face>(face.get());
  if (FT_Load_Glyph(ft_face, static_cast<FT_UInt>(idx), load_flags)) {
    return nullopt;
  }
  const FT_GlyphSlot g = ft_face->glyph;
  // Since SDF glyphs can't be hinted, we have to render them in this phase
  // Otherwise the bitmap glyph metrics will be incorrect
  if (load_mode == ft_mode::sdf && FT_Render_Glyph(g, FT_RENDER_MODE_SDF)) {
    return nullopt;
  }

  const auto& m = g->metrics;
  const auto& b = g->bitmap;
  return ft_glyph_data {
    .id = idx,
    .size = {static_cast<uint32>(m.width>>6), static_cast<uint32>(m.height>>6)},
    .bsize = {static_cast<uint32>(b.width), static_cast<uint32>(b.rows)},
    .hbearing = {static_cast<uint32>(m.horiBearingX>>6), static_cast<uint32>(m.horiBearingY>>6)},
    .vbearing = {static_cast<uint32>(m.vertBearingX>>6), static_cast<uint32>(m.vertBearingY>>6)},
    .bbearing = {static_cast<uint32>(g->bitmap_left), static_cast<uint32>(g->bitmap_top)},
    .advance = {static_cast<uint32>(m.horiAdvance>>6), static_cast<uint32>(m.vertAdvance>>6)},
  };
}

auto ft2_font_loader::_render_bitmap(const face_t& face, uint32 idx,
                                     ft_mode render_mode) -> const uint8* {
  FT_Int32 load_flags;
  FT_Render_Mode ft_render_mode;
  switch (render_mode) {
    case ft_mode::sdf: [[fallthrough]]; // SDF doesn't have a loading hint
    case ft_mode::normal: {
      load_flags = FT_LOAD_TARGET_NORMAL;
      break;
    }
  }
  switch (render_mode) {
    case ft_mode::sdf: {
      ft_render_mode = FT_RENDER_MODE_SDF;
      break;
    }
    case ft_mode::normal: {
      ft_render_mode = FT_RENDER_MODE_NORMAL;
      break;
    }
  }

  FT_Face ft_face = static_cast<FT_Face>(face.get());
  if (FT_Load_Glyph(ft_face, static_cast<FT_UInt>(idx), load_flags)) {
    return nullptr;
  }
  const FT_GlyphSlot g = ft_face->glyph;
  if (g->bitmap.width == 0 || g->bitmap.rows == 0) {
    return nullptr;
  }
  if (FT_Render_Glyph(g, ft_render_mode)) {
    return nullptr;
  }
  return g->bitmap.buffer;
}

// TODO: Use a better packing algorithm
uint32 ft2_font_loader::_find_atlas_extent(uint32 padding, uint32 start_sz,
                                           glyph_metrics* metrics, size_t metric_count) {
  uint32 atlas_extent = start_sz;
  for (;;) {
    uint32 x = padding, y = padding;
    uint32 max_h = 0;
    bool overflows = false;
    for (glyph_metrics* glyph = metrics; glyph != metrics+metric_count; glyph++) {
      const auto gwidth = glyph->size.x;
      const auto gheight = glyph->size.y;
      if (y + gheight + padding > atlas_extent) {
        overflows = true;
        break;
      }

      if (x + gwidth + padding > atlas_extent) {
        x = padding;
        y += 2*padding + max_h;
        max_h = gheight;
      } else {
        max_h = std::max(max_h, gheight);
      }

      // Store the offset here!!!
      glyph->offset.y = y;
      glyph->offset.x = x;

      x += gwidth + padding;
    }
    if (!overflows) {
      break;
    }
    atlas_extent *= 2;
  }
  return atlas_extent;
}

} // namespace ntf
