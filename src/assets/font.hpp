#pragma once

#include "./types.hpp"
#include "./texture.hpp"
#include <ntfstl/hashmap.hpp>

#include <set>
#include <map>

namespace ntf {

template<typename CodeT>
concept font_codepoint_type = meta::same_as_any<CodeT,
  char, wchar_t,
  char8_t, char16_t, char32_t
>;

// TODO: Use an actual set for the charset (lol)
template<font_codepoint_type CodeT, meta::allocator_type<CodeT> Alloc = std::allocator<CodeT>>
using font_charset = std::basic_string<CodeT, std::char_traits<CodeT>, Alloc>;
template<font_codepoint_type CodeT>
using font_charset_view = std::basic_string_view<CodeT, std::char_traits<CodeT>>;

struct glyph_metrics {
  uint32 id;
  ntfr::extent2d size;
  ntfr::extent2d offset;
  ivec2 bearing;
  ivec2 advance;
};

using font_glyphs =
  unique_array<glyph_metrics, allocator_delete<glyph_metrics, virtual_allocator<glyph_metrics>>>;

using glyph_map = fixed_hashmap<
  char32_t, size_t,
  std::hash<char32_t>, std::equal_to<char32_t>,
  allocator_delete<std::pair<const char32_t, size_t>,
    virtual_allocator<std::pair<const char32_t, size_t>>
  >
>;

struct font_atlas_data {
public:
  using bitmap_t = unique_array<uint8, allocator_delete<uint8, virtual_allocator<uint8>>>;

public:
  using iterator = font_glyphs::iterator;
  using const_iterator = font_glyphs::const_iterator;

public:
  font_atlas_data(bitmap_t&& bitmap_, ntfr::extent2d bitmap_extent_,
                  ntfr::image_format bitmap_format_, ntfr::image_alignment bitmap_alignment_,
                  font_glyphs&& glyphs_, glyph_map&& glyph_map_) noexcept :
    bitmap{std::move(bitmap_)}, glyphs{std::move(glyphs_)}, map{std::move(glyph_map_)},
    bitmap_extent{bitmap_extent_}, bitmap_format{bitmap_format_},
    bitmap_alignment{bitmap_alignment_} {}

public:
  template<meta::image_dim_type T = ntfr::extent2d>
  ntfr::image_data make_bitmap_descriptor(
    const T& offset = {}, uint32 layer = 0, uint32 level = 0
  ) const noexcept {
    return ntfr::image_data{
      .bitmap = bitmap.get(),
      .format = bitmap_format,
      .alignment = bitmap_alignment,
      .extent = meta::image_dim_traits<ntfr::extent2d>::extent_cast(bitmap_extent),
      .offset = meta::image_dim_traits<T>::offset_cast(offset),
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
  font_glyphs glyphs;
  glyph_map map;
  ntfr::extent2d bitmap_extent;
  ntfr::image_format bitmap_format;
  ntfr::image_alignment bitmap_alignment;
};

enum class font_load_flags {
  none = 0,
  render_normal = 0,
  render_sdf = 1 << 0,
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(font_load_flags);

class ft2_font_loader {
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
  using temp_map_t = std::map<char32_t, uint32>;
  using temp_set_t = std::set<uint32>;
  using bitmap_t = font_atlas_data::bitmap_t;

private:
  struct ft_glyph_data {
    uint32 id;
    ntfr::extent2d size;  // unused
    ntfr::extent2d bsize;
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
  asset_expected<face_t> _load_face(cspan<uint8> file_data, const ntfr::extent2d& glyph_size);
  uint32 _get_code_index(const face_t& face, uint64 code);
  optional<ft_glyph_data> _load_metrics(const face_t& face, uint32 idx, ft_mode load_mode);
  const uint8* _render_bitmap(const face_t& face, uint32 idx, ft_mode render_mode);

  std::tuple<face_t, font_glyphs, glyph_map> _parse_metrics(
    std::tuple<face_t, temp_map_t, temp_set_t>&& tuple,
    ft_mode mode
  );
  font_atlas_data _load_bitmap(
    std::tuple<face_t, font_glyphs, glyph_map>&& tuple, ft_mode mode,
    uint32 padding, uint32 atlas_size
  );

private:
  static void _unload_lib(void* lib);
  static void _unload_face(void* face);
  static uint32 _find_atlas_extent(uint32 padding, uint32 start_size,
                                   glyph_metrics* metrics, size_t metrics_count);

public:
  template<font_codepoint_type CodeT>
  auto load_atlas(
    cspan<uint8> file_data, font_charset_view<CodeT> charset, font_load_flags flags,
    const ntfr::extent2d& glyph_size, uint32 padding, uint32 atlas_size
  ) -> asset_expected<font_atlas_data>
  {
    const ft_mode mode = +(flags & font_load_flags::render_sdf) ? ft_mode::sdf : ft_mode::normal;
    return _load_face(file_data, glyph_size)
      .transform([this, charset](auto&& face) -> std::tuple<face_t, temp_map_t, temp_set_t> {
        temp_map_t map;
        temp_set_t set;
        for (const auto code : charset) {
          const auto cpoint = static_cast<uint64>(code);
          uint32 idx = _get_code_index(face, cpoint);
          if (!idx) {
            SHOGLE_LOG(verbose, "[ntf::ft2_font_loader] Undefined codepoint '{}'", cpoint);
          }
          set.emplace(idx);
          const auto [_, empl] = map.try_emplace(static_cast<char32_t>(code), idx);
          if (!empl) {
            SHOGLE_LOG(verbose, "[ntf::ft2_font_loader] Duplicate codepoint '{}", cpoint);
          }
        }
        return std::make_tuple(std::move(face), std::move(map), std::move(set));
      })
      .transform([&, this](auto&& tuple) -> std::tuple<face_t, font_glyphs, glyph_map> {
        return _parse_metrics(std::move(tuple), mode);
      })
      .transform([&, this](auto&& parsed_tuple) -> font_atlas_data {
        return _load_bitmap(std::move(parsed_tuple), mode,
                            mode == ft_mode::sdf ? 0u : padding, atlas_size);
      });
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
  font_load_flags flags = font_load_flags::render_sdf,
  uint32 glyph_size = 48u, uint32 padding = 0u,
  font_charset_view<CodeT> charset = ascii_charset<CodeT>,
  Loader&& font_loader = {}
) -> asset_expected<font_atlas_data> {
  SHOGLE_LOG(debug, "[ntf::load_font_atlas] Loading font '{}'", path);
  return file_data(path)
    .and_then([&](auto&& buffer) {
      return font_loader.load_atlas({buffer.get(), buffer.size()}, charset, flags,
                                    {0u, glyph_size}, padding, 128u);
    });
}

} // namespace ntf
