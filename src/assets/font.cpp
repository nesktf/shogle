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

auto ft2_font_loader::_load_face(cspan<uint8> file_data,
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
    .hbearing = {(m.horiBearingX>>6), (m.horiBearingY>>6)},
    .vbearing = {(m.vertBearingX>>6), (m.vertBearingY>>6)},
    .bbearing = {(g->bitmap_left), (g->bitmap_top)},
    .advance = {(m.horiAdvance>>6), (m.vertAdvance>>6)},
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

      x += gwidth + padding;
    }
    if (!overflows) {
      break;
    }
    atlas_extent *= 2;
  }
  return atlas_extent;
}

auto ft2_font_loader::_parse_metrics(
  std::tuple<face_t, temp_map_t, temp_set_t>&& tuple,
  ft_mode mode
) ->  std::tuple<face_t, font_glyphs, glyph_map>
{
  auto&& [face, map, set] = std::move(tuple);

  virtual_allocator<glyph_metrics> metrics_alloc{std::in_place_type_t<malloc_pool>{}};
  virtual_allocator<std::pair<const char32_t, size_t>> map_alloc{std::in_place_type_t<malloc_pool>{}};

  auto parsed_glyphs = font_glyphs::from_size(::ntf::uninitialized,
                                              set.size(),
                                              std::move(metrics_alloc));
  auto parsed_map = glyph_map::from_size(map.size(), std::move(map_alloc)).value();
  auto idx_map = fixed_hashmap<uint32, size_t>::from_size(set.size()).value();

  for (size_t i = 0; const uint32 id : set) {
    auto glyph = _load_metrics(face, id, mode);
    if (!glyph) {
      SHOGLE_LOG(warning, "[ntf::ft2_font_loader] Failed to load glyph with id '{}'",
                 id);
      continue;
    }
    std::construct_at(parsed_glyphs.get()+i, glyph->data_metrics());
    [[maybe_unused]] const auto [_, empl] = idx_map.try_emplace(id, i);
    NTF_ASSERT(empl);
    ++i;
  }
  for (const auto& [code, idx] : map) {
    [[maybe_unused]] const auto [_, empl] = parsed_map.try_emplace(code, idx_map[idx]);
    NTF_ASSERT(empl);
  }

  return std::make_tuple(std::move(face), std::move(parsed_glyphs), std::move(parsed_map));
}

font_atlas_data ft2_font_loader::_load_bitmap(
  std::tuple<face_t, font_glyphs, glyph_map>&& tuple, ft_mode mode,
  uint32 padding, uint32 atlas_size
) {
  auto&& [face, glyphs, map]= std::move(tuple);
  uint32 atlas_extent = _find_atlas_extent(padding, atlas_size,
                                           glyphs.get(), glyphs.size());
  extent2d bitmap_extent{atlas_extent, atlas_extent};
  const size_t bitmap_sz = ntfr::image_stride<uint8>(bitmap_extent);
  virtual_allocator<uint8> bitmap_alloc{std::in_place_type_t<malloc_pool>{}};
  auto* bitmap = bitmap_alloc.allocate(bitmap_sz);
  std::memset(bitmap, 0, bitmap_sz);

  // TODO: Use a better packing algorithm
  uint32 x = padding, y = padding;
  uint32 max_h = 0;
  for (auto& glyph : glyphs) {
    const uint8* buff = _render_bitmap(face, glyph.id, mode);
    if (!buff) {
      SHOGLE_LOG(verbose, "[ntf::ft2_font_loader] Skipping empty bitmap for glyph '{}'",
                 glyph.id);
      continue;
    }
    const uint32 gwidth = glyph.size.x;
    const uint32 gheight = glyph.size.y;
    if (x + gwidth + padding > atlas_extent) {
      x = padding;
      y += 2*padding + max_h;
      max_h = gheight;
    } else {
      max_h = std::max(max_h, gheight);
    }
    
    // Store the offset here!!!
    glyph.offset.x = x;
    glyph.offset.y = y;

    const size_t channels = 1; // TODO: Handle multiple channels for LCD fonts?
    for (size_t row = 0; row < gheight; ++row) {
      const size_t offset = static_cast<size_t>(channels*(x+atlas_extent*(row+y)));
      std::memcpy(bitmap+offset, buff+(row*gwidth), channels*gwidth);
    }

    x += padding + gwidth;
  }

  SHOGLE_LOG(debug,
             "[ntf::ft2_font_loader] Loaded font atlas: {} glyphs, {} mappings, {}x{} bitmap",
             glyphs.size(), map.size(), bitmap_extent.x, bitmap_extent.y);

  using del_t = allocator_delete<uint8, virtual_allocator<uint8>>;
  return font_atlas_data {
    bitmap_t{bitmap_sz, bitmap, del_t{std::move(bitmap_alloc)}},
    bitmap_extent, ntfr::image_format::r8nu, 1u,
    std::move(glyphs), std::move(map)
  };
}

} // namespace ntf
