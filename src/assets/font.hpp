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
struct font_atlas_data {
public:
  using codepoint_type = CharT;

private:
  using bitmap_t = unique_array<uint8, rebind_deleter_t<Deleter, uint8>>;
  using glyphs_t = unique_array<glyph_data<CharT>, rebind_deleter_t<Deleter, glyph_data<CharT>>>;

public:
  using iterator = typename glyphs_t::iterator;
  using const_iterator = typename glyphs_t::const_iterator;

public:
  font_atlas_data(uint8* texels, extent2d extent,
                  r_texture_format format, r_image_alignment alignment,
                  glyph_data<CharT>* glyphs_array, size_t glyphs_sz, uint32 padding,
                  const Deleter& deleter = {}) SHOGLE_ASSET_NOEXCEPT :
    bitmap{texels, tex_stride<uint8>(extent), rebind_deleter_t<Deleter, uint8>{deleter}},
    glyphs{glyphs_array, glyphs_sz, rebind_deleter_t<Deleter, glyph_data<CharT>>{deleter}},
    bitmap_extent{extent}, bitmap_format{format}, bitmap_alignment{alignment},
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
  template<tex_dim_type Dim = extent2d>
  auto make_descriptor(
    const Dim& offset = {}, uint32 layer = 0, uint32 level = 0
  ) const noexcept -> r_image_data
  {
    return r_image_data{
      .texels = bitmap.get(),
      .format = bitmap_format,
      .alignment = bitmap_alignment,
      .extent = ntf::tex_extent_cast(bitmap_extent),
      .offset = ntf::tex_offset_cast(offset),
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
  r_texture_format bitmap_format;
  r_image_alignment bitmap_alignment;
  uint32 glyph_padding;
};
NTF_DEFINE_TEMPLATE_CHECKER(font_atlas_data);

template<
  font_codepoint_type CharT,
  typename Deleter = allocator_delete<CharT, std::allocator<CharT>>
>
struct font_array_data {
public:
  using codepoint_type = CharT;

private:
  using bitmap_t = unique_array<uint8, rebind_deleter_t<Deleter, uint8>>;
  using glyphs_t = unique_array<glyph_data<CharT>, rebind_deleter_t<Deleter, glyph_data<CharT>>>;

public:
  font_array_data(uint8* texels, size_t texels_sz,
                  r_texture_format format, r_image_alignment alignment,
                  glyph_data<CharT>* glyph_array, size_t glyphs_sz,
                  uint32 padding, extent2d extent,
                  const Deleter& deleter = {}) SHOGLE_ASSET_NOEXCEPT :
    bitmap{texels, texels_sz, rebind_deleter_t<Deleter, uint8>{deleter}},
    glyphs{glyph_array, glyphs_sz, rebind_deleter_t<Deleter, glyph_data<CharT>>{deleter}},
    glyph_extent{extent}, bitmap_format{format}, bitmap_alignment{alignment},
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
  template<tex_dim_type Dim = extent2d>
  auto make_descriptor(
    const Dim& offset = {}, uint32 layer = 0, uint32 level = 0
  ) const noexcept -> r_image_data
  {
    return r_image_data{
      .texels = bitmap.get(),
      .format = bitmap_format,
      .alignment = bitmap_alignment,
      .extent = tex_extent_cast(bitmap_extent()),
      .offset = ntf::tex_offset_cast(offset),
      .layer = layer,
      .level = level,
    };
  }

public:
  extent2d bitmap_extent() const noexcept {
    return {glyph_extent.x*glyphs.size(), glyph_extent.y};
  }
  // template<allocator_type<r_image_data> Alloc = std::allocator<r_image_data>>
  // auto make_atlas_descriptor(
  //   uint32 level = 0, Alloc&& alloc = {}
  // ) const SHOGLE_ASSET_NOEXCEPT -> unique_array<r_image_data,allocator_delete<r_image_data,Alloc>>
  // {
  //   auto out = unique_array<r_image_data>::from_allocator(::ntf::uninitialized,
  //                                                         size(),
  //                                                         std::forward<Alloc>(alloc));
  //   SHOGLE_ASSET_THROW_IF(!out.has_data(), "Allocation failure");
  //
  //   size_t offset = 0;
  //   for (const auto& glyph : glyphs) {
  //     // uvec3 glyph_dim{2*glyph_padding + glyph.size.x, 2*glyph_padding + glyph.size.y, 1};
  //     // uvec3 glyph_offset{glyph.offset.x-glyph_padding, glyph.offset.y-glyph_padding, 0};
  //     std::construct_at(out.get()+offset, r_image_data{
  //       .texels = bitmap.get(),
  //       .format = bitmap_format,
  //       .alignment = bitmap_alignment,
  //       .extent = tex_extent_cast(glyph.size),
  //       .offset = tex_array_offset(glyph.offset, offset),
  //       .layer = static_cast<uint32>(offset),
  //       .level = level,
  //     });
  //     offset++;
  //   }
  //
  //   return out;
  // }

public:
  bitmap_t bitmap;
  glyphs_t glyphs;
  extent2d glyph_extent;
  r_texture_format bitmap_format;
  r_image_alignment bitmap_alignment;
  uint32 glyph_padding;
};
NTF_DEFINE_TEMPLATE_CHECKER(font_array_data);

class ft2_font_loader {
public:
  struct face_del_t {
    face_del_t(ft2_font_loader& ft) noexcept :
      loader{ft} {}
    void operator()(void* face) {
      loader._unload_face(face);
    }
    ft2_font_loader& loader;
  };
  friend struct face_del_t;
  using face_t = std::unique_ptr<void, face_del_t>;

  template<typename CharT>
  using map_t = std::map<CharT, glyph_data<CharT>>;

  template<typename CharT>
  using atlas_out_t = font_atlas_data<CharT, allocator_delete<CharT, std::allocator<CharT>>>;

  template<typename CharT>
  using array_out_t = font_array_data<CharT, allocator_delete<CharT, std::allocator<CharT>>>;

  struct ft_glyph_data {
    uvec2 size;
    ivec2 bearing;
    ivec2 advance;
  };

public:
  ft2_font_loader() noexcept;
  ~ft2_font_loader() noexcept;

private:
  void _unload_face(void* face);
  asset_expected<face_t> _load_face(const std::string& path, const uvec2& glyph_size);
  optional<ft_glyph_data> _load_glyph(const face_t& face, uint64 code);
  void _copy_bitmap(const face_t& face, uint64 code, uint8* dest, size_t offset);

  template<typename CharT>
  auto _retrieve_glyphs(font_charset_view<CharT> chars) {
    return [this, chars](auto&& face) -> std::tuple<face_t, map_t<CharT>, extent2d> {
      map_t<CharT> map;

      uint32 max_x = 0;
      uint32 max_y = 0;
      for (const auto code : chars) {
        auto glyph = _load_glyph(face, static_cast<uint64>(code));
        if (!glyph) {
          SHOGLE_LOG(warning, "[ntf::ft2_bitmap_loader] Failed to load codepoint '{}'", code);
          continue;
        }
        max_x = std::max(max_x, glyph->size.x);
        max_y = std::max(max_y, glyph->size.y);
        const auto [it, empl] = map.try_emplace(code, glyph_data<CharT>{
          .id = code,
          .size = glyph->size,
          .offset = {0, 0},
          .bearing = glyph->bearing,
          .advance = glyph->advance,
        });
        if (!empl) {
          SHOGLE_LOG(warning, "[ntf::ft2_bitmap_loader] Ignoring duplicate codepoint '{}'", code);
          continue;
        }
      }

      return std::make_tuple(std::move(face), std::move(map), extent2d{max_x, max_y});
    };
  }

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

public:
  template<font_codepoint_type CharT>
  auto load_atlas(
    const std::string& path, font_charset_view<CharT> charset,
    const extent2d& glyph_size, uint32 padding, uint32 atlas_size
  ) -> asset_expected<atlas_out_t<CharT>>
  {
    return _load_face(path, glyph_size)
    .transform(_retrieve_glyphs<CharT>(charset))
    .transform([this, padding, atlas_size](auto&& pair) -> atlas_out_t<CharT> {
      auto&& [face, map, _] = std::forward<decltype(pair)>(pair);
      const auto atlas_extent = _find_atlas_size(atlas_size, padding, map);
      auto* bitmap = std::allocator<uint8>{}.allocate(atlas_extent*atlas_extent);
      std::memset(bitmap, 0, atlas_extent*atlas_extent);
      auto* glyphs = std::allocator<glyph_data<CharT>>{}.allocate(map.size());

      size_t goff = 0;
      size_t max_h = 0;
      size_t x = 0, y = 0;
      for (auto& [code, glyph] : map) {
        size_t extra_x = glyph.size.x+padding*2;
        size_t extra_y = glyph.size.y+padding*2;
        max_h = std::max(extra_y, max_h);
        if (x + extra_x > atlas_extent) {
          x = 0;
          y += max_h;
          max_h = 0;
        }
        glyph.offset.y = y;
        glyph.offset.x = x;
        std::construct_at(glyphs+goff, std::move(glyph));

        const auto offset = static_cast<size_t>(glyph.offset.y*atlas_extent + glyph.offset.x);
        _copy_bitmap(face, static_cast<uint64>(code), bitmap+offset, atlas_extent);

        ++goff;
        x += extra_x;
      }

      SHOGLE_LOG(debug,
                 "[ntf::ft2_bitmap_loader] Loaded {} glyphs, created {}x{} atlas ({} bytes)",
                 map.size(), atlas_extent, atlas_extent, atlas_extent*atlas_extent);

      return atlas_out_t<CharT> {
        bitmap, extent2d{atlas_extent, atlas_extent},
        r_texture_format::r8nu, r_image_alignment::bytes1,
        glyphs, map.size(), padding,
        allocator_delete<CharT, std::allocator<CharT>>{}
      };
    });
  }

  template<font_codepoint_type CharT>
  auto load_array(
    const std::string& path, font_charset_view<CharT> charset,
    const extent2d& glyph_size
  ) -> asset_expected<array_out_t<CharT>>
  {
    return _load_face(path, glyph_size)
    .transform(_retrieve_glyphs<CharT>(charset))
    .transform([this](auto&& pair) -> array_out_t<CharT> {
      auto&& [face, map, glyph_size] = std::forward<decltype(pair)>(pair);
      const size_t bitmap_sz = glyph_size.x*glyph_size.y*map.size();
      auto* bitmap = std::allocator<uint8>{}.allocate(bitmap_sz);
      std::memset(bitmap, 0, bitmap_sz);
      auto* glyphs = std::allocator<glyph_data<CharT>>{}.allocate(map.size());

      size_t goff = 0;
      for (auto& [code, glyph] : map) {
        std::construct_at(glyphs+goff, std::move(glyph));

        const size_t offset = glyph_size.x*goff;
        _copy_bitmap(face, static_cast<uint64>(code), bitmap+offset, glyph_size.x*map.size());

        ++goff;
      }

      return array_out_t<CharT>{
        bitmap, bitmap_sz,
        r_texture_format::r8nu, r_image_alignment::bytes1,
        glyphs, map.size(),
        0u, glyph_size,
        allocator_delete<CharT, std::allocator<CharT>>{}
      };
    });
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
