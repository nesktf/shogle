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

template<font_codepoint_type CharT>
struct glyph_data {
  CharT id;
  uvec2 size;
  uvec2 offset;
  ivec2 bearing;
  ivec2 advance;
};

template<
  font_codepoint_type CharT,
  typename Deleter = allocator_delete<CharT, std::allocator<CharT>>
>
struct bitmap_font_data {
public:
  using codepoint_type = CharT;

private:
  using bitmap_t = unique_array<uint8, rebind_deleter_t<Deleter, uint8>>;
  using glyphs_t = unique_array<glyph_data<CharT>, rebind_deleter_t<Deleter, glyph_data<CharT>>>;

public:
  using iterator = typename glyphs_t::iterator;
  using const_iterator = typename glyphs_t::const_iterator;

public:
  bitmap_font_data(uint8* texels, uvec2 dim,
                   r_texture_format format, r_image_alignment alignment,
                   glyph_data<CharT>* glyphs_array, size_t glyphs_sz, uint32 padding,
                   const Deleter& deleter = {}) SHOGLE_ASSET_NOEXCEPT :
    bitmap{texels, tex_stride<uint8>(dim), rebind_deleter_t<Deleter, uint8>{deleter}},
    glyphs{glyphs_array, glyphs_sz, rebind_deleter_t<Deleter, glyph_data<CharT>>{deleter}},
    bitmap_dimensions{dim}, bitmap_format{format}, bitmap_alignment{alignment},
    glyph_padding{padding}
  {
    SHOGLE_ASSET_THROW_IF([this](){
      if (!bitmap.has_data() || !glyphs.has_data()) {
        (void)bitmap.release();
        (void)glyphs.release();
        return true;
      }
      return false;
    }(), "Invalid data");
  }

public:
  template<allocator_type<r_image_data> Alloc = std::allocator<r_image_data>>
  auto make_descriptor(
    uint32 level = 0, Alloc&& alloc = {}
  ) const SHOGLE_ASSET_NOEXCEPT -> unique_array<r_image_data,allocator_delete<r_image_data,Alloc>>
  {
    auto out = unique_array<r_image_data>::from_allocator(::ntf::uninitialized,
                                                          size(),
                                                          std::forward<Alloc>(alloc));
    SHOGLE_ASSET_THROW_IF(!out.has_data(), "Allocation failure");

    size_t offset = 0;
    for (const auto& glyph : glyphs) {
      // uvec3 glyph_dim{2*glyph_padding + glyph.size.x, 2*glyph_padding + glyph.size.y, 1};
      // uvec3 glyph_offset{glyph.offset.x-glyph_padding, glyph.offset.y-glyph_padding, 0};
      std::construct_at(out.get()+offset, r_image_data{
        .texels = bitmap.get(),
        .format = bitmap_format,
        .alignment = bitmap_alignment,
        .extent = tex_extent_cast(glyph.size),
        .offset = tex_array_offset(glyph.offset, offset),
        .layer = static_cast<uint32>(offset),
        .level = level,
      });
      offset++;
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
  uvec2 bitmap_dimensions;
  r_texture_format bitmap_format;
  r_image_alignment bitmap_alignment;
  uint32 glyph_padding;
};
NTF_DEFINE_TEMPLATE_CHECKER(bitmap_font_data);

class ft2_bitmap_loader {
public:
  struct face_del_t {
    face_del_t(ft2_bitmap_loader& ft) noexcept :
      loader{ft} {}
    void operator()(void* face) {
      loader._unload_face(face);
    }
    ft2_bitmap_loader& loader;
  };
  friend struct face_del_t;
  using face_t = std::unique_ptr<void, face_del_t>;

  template<typename CharT>
  using map_t = std::map<CharT, glyph_data<CharT>>;

  template<typename CharT>
  using out_t = bitmap_font_data<CharT, allocator_delete<CharT, std::allocator<CharT>>>;

  struct ft_glyph_data {
    uvec2 size;
    ivec2 bearing;
    ivec2 advance;
  };

public:
  ft2_bitmap_loader() noexcept;
  ~ft2_bitmap_loader() noexcept;

private:
  void _unload_face(void* face);
  asset_expected<face_t> _load_face(const std::string& path, const uvec2& glyph_size);
  optional<ft_glyph_data> _load_glyph(const face_t& face, uint64 code);
  void _copy_bitmap(const face_t& face, uint64 code, uint8* dest, size_t offset);

  template<typename CharT>
  uint32 _find_atlas_size(uint32 min, uint32 pad, const map_t<CharT>& map) {
    auto sz = min;
    auto is_enough = [&]() {
      uint32 x = 0, y = 0;
      uint32 max_h = 0;
      for (const auto& [_, glyph] : map) {
        const auto gwidth = glyph.size.x + pad*2;
        const auto gheight = glyph.size.y + pad*2;
        max_h = std::max(max_h, gheight);
        if (x + gwidth > sz) {
          x = 0;
          y += max_h;
          max_h = gheight;
        }
        if (y + gheight > sz) {
          return false;
        }
        x += gwidth;
      }
      return true;
    };
    while (!is_enough()) {
      sz *= 2;
    }
    return sz;
  }

  template<typename CharT>
  auto _retrieve_glyphs(font_charset_view<CharT> chars, uint32 padding) {
    using tuple_t = std::tuple<face_t, map_t<CharT>>;
    return [this, chars, padding](auto&& face) -> asset_expected<tuple_t> {
      map_t<CharT> map;

      for (const auto code : chars) {
        auto glyph = _load_glyph(face, static_cast<uint64>(code));
        if (!glyph) {
          SHOGLE_LOG(warning, "[ntf::ft2_bitmap_loader] Failed to load codepoint '{}'", code);
          continue;
        }
        const auto [it, empl] = map.try_emplace(code, glyph_data<CharT>{
          .id = code,
          .size = glyph->size,
          .offset = {},
          .bearing = glyph->bearing,
          .advance = glyph->advance,
        });
        if (!empl) {
          SHOGLE_LOG(warning, "[ntf::ft2_bitmap_loader] Ignoring duplicate codepoint '{}'", code);
          continue;
        }
      }

      return std::make_tuple(std::move(face), std::move(map));
    };
  }

  template<typename CharT>
  auto _create_bitmap(uint32 padding, uint32 atlas_size) {
    return [this, padding, atlas_size](auto&& pair) -> asset_expected<out_t<CharT>> {
      auto&& [face, map] = std::forward<decltype(pair)>(pair);
      const auto sz = _find_atlas_size(atlas_size, padding, map);
      auto* bitmap = std::allocator<uint8>{}.allocate(sz*sz);
      std::memset(bitmap, 0, sz*sz);
      auto* glyphs = std::allocator<glyph_data<CharT>>{}.allocate(map.size());

      size_t goff = 0;

      size_t max_h = 0;
      size_t x = 0, y = 0;
      for (auto& [code, glyph] : map) {
        size_t extra_x = glyph.size.x+padding*2;
        size_t extra_y = glyph.size.y+padding*2;
        max_h = std::max(extra_y, max_h);
        if (x + extra_x > sz) {
          x = 0;
          y += max_h;
          max_h = 0;
        }
        glyph.offset.y = y;
        glyph.offset.x = x;
        const auto start = static_cast<size_t>(glyph.offset.y*sz + glyph.offset.x);
        _copy_bitmap(face, static_cast<uint64>(code), bitmap+start, sz);
        std::construct_at(glyphs+goff, std::move(glyph));
        ++goff;
        x += extra_x;
      }

      SHOGLE_LOG(debug,
                 "[ntf::ft2_bitmap_loader] Loaded {} glyphs, created {}x{} atlas ({} bytes)",
                 map.size(), sz, sz, sz*sz);

      return out_t<CharT> {
        bitmap, uvec2{sz, sz},
        r_texture_format::r8nu, r_image_alignment::bytes1,
        glyphs, map.size(), padding,
        allocator_delete<CharT, std::allocator<CharT>>{}
      };
    };
  }

public:
  template<font_codepoint_type CharT>
  auto load(
    const std::string& path, font_charset_view<CharT> charset,
    const uvec2& glyph_size, uint32 padding, uint32 atlas_size
  ) -> asset_expected<out_t<CharT>>
  {
    return _load_face(path, glyph_size)
      .and_then(_retrieve_glyphs<CharT>(charset, padding))
      .and_then(_create_bitmap<CharT>(padding, atlas_size));
  }

private:
  void* _ft2_lib;
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
