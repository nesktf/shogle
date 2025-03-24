#include "./font.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#define RET_ERR(msg, ...) \
  SHOGLE_LOG(error, "[ntf::ft2_bitmap_loader] " msg __VA_OPT__(,) __VA_ARGS__); \
  return unexpected{asset_error::format({msg}__VA_OPT__(,) __VA_ARGS__)};

#define RET_ERR_IF(cond, ...) \
  if (cond) { RET_ERR(__VA_ARGS__); }

namespace ntf {

ft2_font_loader::ft2_font_loader() noexcept {
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

ft2_font_loader::~ft2_font_loader() noexcept {
  if (!_ft2_lib) {
    return;
  }
  FT_Done_FreeType(static_cast<FT_Library>(_ft2_lib));
}

auto ft2_font_loader::_load_face(const std::string& path,
                                   const uvec2& glyph_size) -> asset_expected<face_t> {
  RET_ERR_IF(!_ft2_lib, "Failed to initialize FreeType");
  FT_Library ft = static_cast<FT_Library>(_ft2_lib);

  FT_Face face;
  RET_ERR_IF(FT_New_Face(ft, path.c_str(), 0, &face), "Failed to load font");

  RET_ERR_IF(FT_Set_Pixel_Sizes(face, glyph_size.x, glyph_size.y),
             "Failed to set pixel size to {} {}",
             glyph_size.x, glyph_size.y);

  static_assert(std::is_pointer_v<FT_Face>); // Assume FT_Face is just a pointer handle
  return face_t{face, face_del_t{*this}};
}

void ft2_font_loader::_unload_face(void* face) {
  FT_Done_Face(static_cast<FT_Face>(face));
}

auto ft2_font_loader::_load_glyph(const face_t& face, uint64 code) -> optional<ft_glyph_data> {
  FT_Face ft_face = static_cast<FT_Face>(face.get());
  if (FT_Load_Char(ft_face, static_cast<FT_ULong>(code), FT_LOAD_BITMAP_METRICS_ONLY)) {
    return nullopt;
  }
  auto ret_render = FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_SDF);
  NTF_ASSERT(!ret_render);

  const auto* g = ft_face->glyph;
  return ft_glyph_data{
    .size = {static_cast<uint32>(g->bitmap.width), static_cast<uint32>(g->bitmap.rows)},
    .offset = {0, 0},
    .bearing {static_cast<int32>(g->bitmap_left), static_cast<int32>(g->bitmap_top)},
    .advance = {static_cast<int32>(g->advance.x>>6), static_cast<int32>(g->advance.y>>6)},
  };
}

void ft2_font_loader::_copy_bitmap(const face_t& face, uint64 code, uint8* dest, size_t offset) {
  FT_Face ft_face = static_cast<FT_Face>(face.get());
  auto ret_load = FT_Load_Char(ft_face, static_cast<FT_ULong>(code), FT_LOAD_DEFAULT);
  NTF_ASSERT(!ret_load);
  auto ret_render = FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_SDF);
  NTF_ASSERT(!ret_render);

  const auto& bm = ft_face->glyph->bitmap;
  for (size_t row = 0; row < bm.rows; ++row) {
    std::memcpy(dest+(offset*row), bm.buffer+(row*bm.width), bm.width);
  } 
}

} // namespace ntf
