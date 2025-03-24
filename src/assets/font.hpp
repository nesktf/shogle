#pragma once

#include "./types.hpp"
#include "./texture.hpp"

namespace ntf {

template<typename CharT>
concept font_codepoint_type = same_as_any<CharT,
  char, wchar_t,
  char8_t, char16_t, char32_t
>;

template<font_codepoint_type CharT, allocator_type<CharT> Alloc = std::allocator<CharT>>
using font_charset = std::basic_string<CharT, std::char_traits<CharT>, Alloc>;

template<font_codepoint_type CharT>
using font_charset_view = std::basic_string_view<CharT, std::char_traits<CharT>>;

template<
  font_codepoint_type CharT,
  typename Deleter = allocator_delete<CharT, std::allocator<CharT>>
>
struct font_atlas_data {
public:
  using codepoint_type = CharT;

  struct glyph_data {
    CharT id;
    extent2d size;
    extent2d offset;
    ivec2 bearing;
    ivec2 advance;
  };

private:
  using bitmap_t = unique_array<uint8, rebind_deleter_t<Deleter, uint8>>;
  using glyphs_t = unique_array<glyph_data, rebind_deleter_t<Deleter, glyph_data>>;

public:
  using iterator = typename glyphs_t::iterator;
  using const_iterator = typename glyphs_t::const_iterator;

public:
  font_atlas_data(uint8* bitmap_, extent2d bitmap_extent_,
                  r_texture_format bitmap_format_, r_image_alignment bitmap_alignment_,
                  glyph_data* glyphs_, size_t glyph_count_, uint32 glyph_padding_,
                  const Deleter& deleter_ = {}) noexcept :
    bitmap{bitmap_, tex_stride<uint8>(bitmap_extent_), rebind_deleter_t<Deleter, uint8>{deleter_}},
    glyphs{glyphs_, glyph_count_, rebind_deleter_t<Deleter, glyph_data>{deleter_}},
    bitmap_extent{bitmap_extent_}, glyph_padding{glyph_padding_},
    bitmap_format{bitmap_format_}, bitmap_alignment{bitmap_alignment_} {}

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
  extent2d bitmap_extent;
  uint32 glyph_padding;
  r_texture_format bitmap_format;
  r_image_alignment bitmap_alignment;
};
NTF_DEFINE_TEMPLATE_CHECKER(font_atlas_data);

template<
  font_codepoint_type CharT,
  typename Deleter = allocator_delete<CharT, std::allocator<CharT>>
>
struct font_array_data {
public:
  using codepoint_type = CharT;

  struct glyph_data {
    CharT id;
    extent2d size;
    ivec2 bearing;
    ivec2 advance;
  };

private:
  using bitmap_t = unique_array<uint8, rebind_deleter_t<Deleter, uint8>>;
  using glyphs_t = unique_array<glyph_data, rebind_deleter_t<Deleter, glyph_data>>;

public:
  using iterator = typename glyphs_t::iterator;
  using const_iterator = typename glyphs_t::const_iterator;

public:
  font_array_data(uint8* bitmap_, extent2d bitmap_extent_,
                  r_texture_format bitmap_format_, r_image_alignment bitmap_alignment_,
                  glyph_data* glyphs_, size_t glyph_count_, extent2d glyph_extent_,
                  const Deleter& deleter_ = {}) noexcept :
    bitmap{bitmap_, tex_stride<uint8>(bitmap_extent_), rebind_deleter_t<Deleter, uint8>{deleter_}},
    glyphs{glyphs_, glyph_count_, rebind_deleter_t<Deleter, glyph_data>{deleter_}},
    bitmap_extent{bitmap_extent_}, glyph_extent{glyph_extent_},
    bitmap_format{bitmap_format_}, bitmap_alignment{bitmap_alignment_} {}

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

  template<allocator_type<r_image_data> Alloc = std::allocator<r_image_data>>
  auto make_array_descriptor(
    uint32 level = 0, Alloc&& alloc = {}
  ) const SHOGLE_ASSET_NOEXCEPT -> unique_array<r_image_data, allocator_delete<r_image_data,Alloc>>
  {
    auto out = unique_array<r_image_data>::from_allocator(::ntf::uninitialized,
                                                          size(),
                                                          std::forward<Alloc>(alloc));
    SHOGLE_ASSET_THROW_IF(!out.has_data(), "Allocation failure");

    size_t offset = 0;
    for (const auto& glyph : glyphs) {
      std::construct_at(out.get()+offset, r_image_data{
        .texels = bitmap.get(),
        .format = bitmap_format,
        .alignment = bitmap_alignment,
        .extent = tex_extent_cast(glyph_extent),
        .offset = {0, 0, static_cast<uint32>(offset)},
        .layer = static_cast<uint32>(offset),
        .level = level,
      });
      ++offset;
    }

    return out;
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
  extent2d bitmap_extent;
  extent2d glyph_extent;
  r_texture_format bitmap_format;
  r_image_alignment bitmap_alignment;
};
NTF_DEFINE_TEMPLATE_CHECKER(font_array_data);

class ft2_font_loader {
private:
  struct face_del_t {
    void operator()(void* face) {
      ft2_font_loader::_unload_face(face);
    }
  };
  friend struct face_del_t;
  using face_t = std::unique_ptr<void, face_del_t>;

  struct lib_del_t {
    void operator()(void* lib) {
      ft2_font_loader::_unload_lib(lib);
    }
  };
  friend struct lib_del_t;
  using lib_t = std::unique_ptr<void, lib_del_t>;

  template<typename CharT>
  using atlas_out_t = font_atlas_data<CharT, allocator_delete<CharT, std::allocator<CharT>>>;

  template<typename CharT>
  using array_out_t = font_array_data<CharT, allocator_delete<CharT, std::allocator<CharT>>>;

public:
  struct ft_glyph_data {
    extent2d size;
    extent2d offset;
    ivec2 bearing;
    ivec2 advance;
  };

public:
  ft2_font_loader() noexcept;

private:
  asset_expected<face_t> _load_face(const std::string& path, const extent2d& glyph_size);
  optional<ft_glyph_data> _load_glyph(const face_t& face, uint64 code);
  void _copy_bitmap(const face_t& face, uint64 code, uint8* dest, size_t offset);

private:
  static void _unload_lib(void* lib);
  static void _unload_face(void* face);

private:
  template<typename CharT>
  auto _retrieve_glyphs(font_charset_view<CharT> charset) {
    using map_t = std::map<CharT, ft_glyph_data>;
    return [this, charset](auto&& face) -> std::tuple<face_t, map_t, extent2d, uint32> {
      map_t map;
      uint32 max_x = 0;
      uint32 max_y = 0;
      uint32 non_empties = 0;
      for (const auto code : charset) {
        auto glyph = _load_glyph(face, static_cast<uint64>(code));
        if (!glyph) {
          SHOGLE_LOG(warning, "[ntf::ft2_font_loader] Failed to load codepoint '{}'", code);
          continue;
        }
        const auto [it, empl] = map.try_emplace(code, *glyph);
        if (!empl) {
          SHOGLE_LOG(warning, "[ntf::ft2_font_loader] Ignoring duplicate codepoint '{}'", code);
          continue;
        }
        if (glyph->size.x != 0 && glyph->size.y != 0) {
          non_empties++;
        }
        max_x = std::max(max_x, glyph->size.x);
        max_y = std::max(max_y, glyph->size.y);
      }
      return std::make_tuple(std::move(face), std::move(map), extent2d{max_x, max_y}, non_empties);
    };
  }

  template<typename CharT>
  auto _find_atlas_extent(uint32 padding, uint32 atlas_size) {
    using map_t = std::map<CharT, ft_glyph_data>;
    return [padding, atlas_size](auto&& glyphs) -> std::tuple<face_t, map_t, uint32> {
      auto&& [face, map, _, __] = std::forward<decltype(glyphs)>(glyphs);
      uint32 atlas_extent = atlas_size;
      for (;;) {
        uint32 x = 0, y = 0;
        uint32 max_h = 0;
        bool overflows = false;
        for (auto& [_, glyph] : map) {
          const auto gwidth = glyph.size.x+padding*2;
          const auto gheight = glyph.size.y+padding*2;
          if (y + gheight > atlas_extent) {
            overflows = true;
            break;
          }

          max_h = std::max(max_h, gheight);
          if (x + gwidth > atlas_extent) {
            x = 0;
            y += max_h;
            max_h = gheight;
          }

          // Store the offset too!
          glyph.offset.y = y;
          glyph.offset.x = x;

          x += gwidth;
        }
        if (!overflows) {
          break;
        }
        atlas_extent *= 2;
      }

      return std::make_tuple(std::move(face), std::move(map), atlas_extent);
    };
  }

  template<typename CharT>
  auto _build_atlas_bitmap(uint32 padding) {
    using gdata_t = typename atlas_out_t<CharT>::glyph_data;
    return [this, padding](auto&& glyphs) -> atlas_out_t<CharT> {
      auto&& [face, map, atlas_extent] = std::forward<decltype(glyphs)>(glyphs);

      extent2d bitmap_extent{atlas_extent, atlas_extent};
      const size_t bitmap_sz = tex_stride<uint8>(bitmap_extent);
      auto* bitmap = std::allocator<uint8>{}.allocate(bitmap_sz);
      auto* glyphs_out = std::allocator<gdata_t>{}.allocate(map.size());
      std::memset(bitmap, 0, bitmap_sz);

      size_t glyph_count = 0;
      for (auto& [code, glyph] : map) {
        std::construct_at(glyphs_out+glyph_count, gdata_t{
          .id = code,
          .size = {glyph.size.x+padding*2, glyph.size.y+padding*2},
          .offset = glyph.offset,
          .bearing = glyph.bearing,
          .advance = glyph.advance,
        });

        if (glyph.size.x != 0 && glyph.size.y != 0) {
          const auto offset = static_cast<size_t>(glyph.offset.y*atlas_extent + glyph.offset.x);
          _copy_bitmap(face, static_cast<uint64>(code), bitmap+offset, bitmap_extent.x);
        } else {
          SHOGLE_LOG(verbose, "[ntf::ft2_font_loader] Skipping empty bitmap for codepoint '{}'",
                     static_cast<uint64>(code));
        }

        ++glyph_count;
      }

      SHOGLE_LOG(debug,
                 "[ntf::ft2_font_loader] Loaded font atlas with {} glyphs, created {}x{} bitmap",
                 map.size(), bitmap_extent.x, bitmap_extent.y);

      return atlas_out_t<CharT> {
        bitmap, bitmap_extent,
        r_texture_format::r8nu, r_image_alignment::bytes1,
        glyphs_out, map.size(), padding,
        allocator_delete<CharT, std::allocator<CharT>>{}
      };
    };
  }

  template<typename CharT>
  auto _build_array_bitmap() {
    using gdata_t = typename array_out_t<CharT>::glyph_data;
    return [this](auto&& glyphs) -> array_out_t<CharT> {
      auto&& [face, map, glyph_extent, non_empties] = std::forward<decltype(glyphs)>(glyphs);

      extent2d bitmap_extent{glyph_extent.x*non_empties, glyph_extent.y};
      const size_t bitmap_sz = tex_stride<uint8>(bitmap_extent);
      auto* bitmap = std::allocator<uint8>{}.allocate(bitmap_sz);
      auto* glyphs_out = std::allocator<gdata_t>{}.allocate(map.size());
      std::memset(bitmap, 0, bitmap_sz);

      size_t glyph_count = 0;
      size_t bitmap_count = 0;
      for (auto& [code, glyph] : map) {
        std::construct_at(glyphs_out+glyph_count, gdata_t{
          .id = code,
          .size = glyph.size,
          .bearing = glyph.bearing,
          .advance = glyph.advance,
        });

        if (glyph.size.x != 0 && glyph.size.y != 0) {
          const size_t offset = glyph_extent.x*bitmap_count;
          _copy_bitmap(face, static_cast<uint64>(code), bitmap+offset, bitmap_extent.x);
          ++bitmap_count;
        } else {
          SHOGLE_LOG(verbose, "[ntf::ft2_font_loader] Skipping empty bitmap for codepoint '{}'",
                     static_cast<uint64>(code));
        }

        ++glyph_count;
      }

      SHOGLE_LOG(debug,
                 "[ntf::ft2_font_loader] Loaded font array with {} glyphs, created {}x{} bitmap",
                 map.size(), bitmap_extent.x, bitmap_extent.y);

      return array_out_t<CharT>{
        bitmap, bitmap_extent,
        r_texture_format::r8nu, r_image_alignment::bytes1,
        glyphs_out, map.size(), glyph_extent,
        allocator_delete<CharT, std::allocator<CharT>>{}
      };
    };
  }

public:
  template<font_codepoint_type CharT>
  auto load_atlas(
    const std::string& path, font_charset_view<CharT> charset,
    const extent2d& glyph_size, uint32 padding, uint32 atlas_size
  ) -> asset_expected<atlas_out_t<CharT>>
  {
    return _load_face(path, glyph_size)
      .transform(_retrieve_glyphs<CharT>(charset))
      .transform(_find_atlas_extent<CharT>(padding, atlas_size))
      .transform(_build_atlas_bitmap<CharT>(padding));
  }

  template<font_codepoint_type CharT>
  auto load_array(
    const std::string& path, font_charset_view<CharT> charset,
    const extent2d& glyph_size
  ) -> asset_expected<array_out_t<CharT>>
  {
    return _load_face(path, glyph_size)
      .transform(_retrieve_glyphs<CharT>(charset))
      .transform(_build_array_bitmap<CharT>());
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

} // namespace ntf
