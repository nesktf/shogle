#pragma once

#include "./types.hpp"
#include "./texture.hpp"
#include "../stl/hashmap.hpp"

#include <set>

namespace ntf {

template<typename CodeT>
concept font_codepoint_type = same_as_any<CodeT,
  char, wchar_t,
  char8_t, char16_t, char32_t
>;

// TODO: Use an actual set for the charset (lol)
template<font_codepoint_type CodeT, allocator_type<CodeT> Alloc = std::allocator<CodeT>>
using font_charset = std::basic_string<CodeT, std::char_traits<CodeT>, Alloc>;
template<font_codepoint_type CodeT>
using font_charset_view = std::basic_string_view<CodeT, std::char_traits<CodeT>>;

struct glyph_metrics {
  uint32 id;
  extent2d size;
  extent2d offset;
  ivec2 bearing;
  ivec2 advance;
};

template<
  font_codepoint_type CodeT,
  allocator_type<CodeT> AllocT = std::allocator<CodeT>>
struct font_atlas_data {
public:
  using codepoint_type = CodeT;
  using allocator_type = AllocT;

private:
  using bitmap_t = unique_array_alloc<uint8, AllocT>;
  using glyphs_t = unique_array_alloc<glyph_metrics, AllocT>;
  using map_t = fixed_hashmap_alloc<CodeT, size_t, AllocT>;

public:
  using iterator = typename glyphs_t::iterator;
  using const_iterator = typename glyphs_t::const_iterator;

public:
  font_atlas_data(bitmap_t bitmap_, extent2d bitmap_extent_,
                  r_texture_format bitmap_format_, r_image_alignment bitmap_alignment_,
                  glyphs_t glyphs_, map_t glyph_map_, uint32 glyph_padding_) noexcept :
    bitmap{std::move(bitmap_)}, glyphs{std::move(glyphs_)}, glyph_map{std::move(glyph_map_)},
    bitmap_extent{bitmap_extent_}, bitmap_format{bitmap_format_},
    bitmap_alignment{bitmap_alignment_}, glyph_padding{glyph_padding_} {}

public:
  template<tex_dim_type Dim = extent2d>
  auto make_bitmap_descriptor(
    const Dim& offset = {}, uint32 layer = 0, uint32 level = 0
  ) const noexcept -> r_image_data
  {
    return r_image_data{
      .texels = bitmap.get(),
      .format = bitmap_format,
      .alignment = bitmap_alignment,
      .extent = tex_extent_cast(bitmap_extent),
      .offset = tex_offset_cast(offset),
      .layer = layer,
      .level = level,
    };
  }

public:
  size_t size() const noexcept { return glyphs.size(); }

  iterator begin() noexcept { return glyphs.begin(); }
  const_iterator begin() const noexcept { return glyphs.begin(); }
  const_iterator cbegin() const noexcept { return glyphs.cbegin(); }

  iterator end() noexcept { return glyphs.end(); }
  const_iterator end() const noexcept { return glyphs.end(); }
  const_iterator cend() const noexcept { return glyphs.cend(); }

public:
  bitmap_t bitmap;
  glyphs_t glyphs;
  map_t glyph_map;
  extent2d bitmap_extent;
  r_texture_format bitmap_format;
  r_image_alignment bitmap_alignment;
  uint32 glyph_padding;
};
NTF_DEFINE_TEMPLATE_CHECKER(font_atlas_data);

enum class font_load_flags {
  none = 0,
  render_normal = 0,
  render_sdf = 1 << 0,
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(font_load_flags);

class ft2_font_loader {
public:
  template<typename T>
  using data_alloc = std::allocator<T>;

private:
  friend struct face_del_t;
  friend struct lib_del_t;
  struct face_del_t {
    void operator()(void* face) { ft2_font_loader::_unload_face(face); }
  };
  struct lib_del_t {
    void operator()(void* lib) { ft2_font_loader::_unload_lib(lib); }
  };
  using face_t = std::unique_ptr<void, face_del_t>;
  using lib_t = std::unique_ptr<void, lib_del_t>;

private:

  template<typename CodeT>
  using atlas_out_t = font_atlas_data<CodeT, data_alloc<CodeT>>;

  struct ft_glyph_data {
    uint32 id;
    extent2d size;  // unused
    extent2d bsize;
    ivec2 hbearing; // unused
    ivec2 vbearing; // unused
    ivec2 bbearing;
    ivec2 advance;

    auto data_metrics() const -> glyph_metrics {
      // TODO: do something with the original bearing and size metrics?
      return {
        .id = id,
        .size = bsize,
        .offset = {0u, 0u}, // Set later
        .bearing = bbearing,
        .advance = advance,
      };
    }
  };

  enum class ft_mode {
    normal = 0,
    sdf,
  };

public:
  ft2_font_loader() noexcept;

private:
  asset_expected<face_t> _load_face(span_view<uint8> file_data, const extent2d& glyph_size);
  uint32 _get_code_index(const face_t& face, uint64 code);
  optional<ft_glyph_data> _load_metrics(const face_t& face, uint32 idx, ft_mode load_mode);
  const uint8* _render_bitmap(const face_t& face, uint32 idx, ft_mode render_mode);

private:
  static void _unload_lib(void* lib);
  static void _unload_face(void* face);
  static uint32 _find_atlas_extent(uint32 padding, uint32 start_size,
                                   glyph_metrics* metrics, size_t metrics_count);

private:
  template<typename CodeT>
  auto _parse_codepoints(font_charset_view<CodeT> charset) {
    using map_t = std::map<CodeT, uint32>;
    using set_t = std::set<uint32>;
    return [this, charset](auto&& face) -> std::tuple<face_t, map_t, set_t> {
      map_t map;
      set_t set;
      for (const auto code : charset) {
        const auto cpoint = static_cast<uint64>(code);
        uint32 idx = _get_code_index(face, cpoint);
        if (!idx) {
          SHOGLE_LOG(verbose, "[ntf::ft2_font_loader] Undefined codepoint '{}'", cpoint);
        }
        set.emplace(idx);
        const auto [_, empl] = map.try_emplace(code, idx);
        if (!empl) {
          SHOGLE_LOG(verbose, "[ntf::ft2_font_loader] Duplicate codepoint '{}", cpoint);
        }
      }
      return std::make_tuple(std::move(face), std::move(map), std::move(set));
    };
  }

  template<typename CodeT>
  auto _parse_glyphs(ft_mode mode) {
    using glyphs_t = unique_array_alloc<glyph_metrics, data_alloc<glyph_metrics>>;
    using map_t = fixed_hashmap_alloc<CodeT, size_t, data_alloc<CodeT>>;

    return [this, mode](auto&& cpoints_tuple) -> std::tuple<face_t, glyphs_t, map_t> {
      auto&& [face, map, set] = std::forward<decltype(cpoints_tuple)>(cpoints_tuple);
      data_alloc<glyph_metrics> alloc;

      auto parsed_glyphs = unique_array<glyph_metrics>::from_allocator(::ntf::uninitialized,
                                                                       set.size(), alloc);
      map_t parsed_map{map.size()};

      fixed_hashmap_alloc<uint32, size_t, data_alloc<uint32>> idx_map{set.size()};
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
    };
  }

  template<typename CodeT>
  auto _render_bitmap(uint32 padding, uint32 atlas_size, ft_mode mode) {
    return [this, padding, atlas_size, mode](auto&& parsed_tuple) -> atlas_out_t<CodeT> {
      auto&& [face, glyphs, map] = std::forward<decltype(parsed_tuple)>(parsed_tuple);
      uint32 atlas_extent = _find_atlas_extent(padding, atlas_size,
                                               glyphs.get(), glyphs.size());
      extent2d bitmap_extent{atlas_extent, atlas_extent};
      const size_t bitmap_sz = tex_stride<uint8>(bitmap_extent);
      auto* bitmap = data_alloc<uint8>{}.allocate(bitmap_sz);
      std::memset(bitmap, 0, bitmap_sz);

      // TODO: Use a better packing algorithm
      uint32 x = padding, y = padding;
      uint32 max_h = 0;
      for (const auto& glyph : glyphs) {
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

      return atlas_out_t<CodeT> {
        unique_array_alloc<uint8, data_alloc<uint8>>{bitmap, bitmap_sz}, bitmap_extent, 
        r_texture_format::r8nu, r_image_alignment::bytes1,
        std::move(glyphs), std::move(map), padding
      };
    };
  }

public:
  template<font_codepoint_type CodeT>
  auto load_atlas(
    span_view<uint8> file_data, font_charset_view<CodeT> charset, font_load_flags flags,
    const extent2d& glyph_size, uint32 padding, uint32 atlas_size
  ) -> asset_expected<atlas_out_t<CodeT>>
  {
    const ft_mode mode = +(flags & font_load_flags::render_sdf) ? ft_mode::sdf : ft_mode::normal;
    return _load_face(file_data, glyph_size)
      .transform(_parse_codepoints<CodeT>(charset))
      .transform(_parse_glyphs<CodeT>(mode))
      .transform(_render_bitmap<CodeT>(mode == ft_mode::sdf ? 0u : padding, atlas_size, mode));
  }

private:
  lib_t _ft2_lib;
};

template<typename T, T a, T b>
constexpr auto array_range = []() {
  std::array<T, b-a+1> data{};
  for (size_t c = static_cast<size_t>(a); c <= static_cast<size_t>(b); ++c) {
    data[c-a] = c;
  }
  return data;
}();

template<font_codepoint_type T>
constexpr font_charset_view<T> ascii_charset{array_range<T, T{0}, T{127}>.data(), 128u};

template<font_codepoint_type CodeT, typename Loader = ntf::ft2_font_loader>
auto load_font_atlas(
  const std::string& path,
  font_load_flags flags = font_load_flags::render_normal,
  uint32 glyph_size = 32u, uint32 padding = 2u,
  font_charset_view<CodeT> charset = ascii_charset<CodeT>,
  Loader&& font_loader = {}
) -> asset_expected<font_atlas_data<
  CodeT, typename std::remove_cvref_t<Loader>::template data_alloc<CodeT>
>> {
  SHOGLE_LOG(debug, "[ntf::load_font_atlas] Loading font '{}'", path);
  return file_data(path)
    .and_then([&](auto&& buffer) {
      return font_loader.load_atlas({buffer.get(), buffer.size()}, charset, flags,
                                    {0u, glyph_size}, padding, 128u);
    });
}

} // namespace ntf
