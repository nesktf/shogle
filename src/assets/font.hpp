#pragma once

#include "./types.hpp"
#include "./texture.hpp"

// #include "../render/forward.hpp"

namespace ntf {

template<typename CharT>
concept glyph_char_type = same_as_any<std::remove_cvref_t<CharT>,
  char, wchar_t,
  char8_t, char16_t, char32_t
>;

template<glyph_char_type CharT, allocator_type<CharT> Alloc = std::allocator<CharT>>
using font_charset = std::basic_string<CharT, std::char_traits<CharT>, Alloc>;

template<glyph_char_type CharT>
using font_charset_view = std::basic_string_view<CharT, std::char_traits<CharT>>;

template<glyph_char_type CharT>
struct glyph_data {
  CharT id;
  size_t offset;
  uvec2 size;
  ivec2 bearing;
  ivec2 advance;
};

template<
  glyph_char_type CharT,
  tex_depth_type DepthT,
  typename GlyphDeleter = allocator_delete<glyph_data<CharT>, std::allocator<glyph_data<CharT>>>,
  typename TexelDeleter = allocator_delete<DepthT, std::allocator<DepthT>>
>
struct bitmap_font_data {
public:
  using char_type = CharT;
  using depth_type = DepthT;

  using texel_data_type = unique_array<depth_type, TexelDeleter>;
  using glyph_data_type = unique_array<glyph_data<CharT>, GlyphDeleter>;

public:
  bitmap_font_data(texel_data_type&& texel_array_, glyph_data_type&& glyphs_,
                   r_texture_format format_,
                   r_image_alignment alignment_) SHOGLE_ASSET_NOEXCEPT :
    texel_array{std::move(texel_array_)}, glyphs{std::move(glyphs_)},
    format{format_}, alignment{alignment_}
  {
    SHOGLE_ASSET_THROW_IF(!texel_array.has_data(), "Invalid texel data");
    SHOGLE_ASSET_THROW_IF(!glyphs.has_data(), "Invalid glyph data");
  }

public:
  texel_data_type texel_array;
  glyph_data_type glyphs;
  r_texture_format format;
  r_image_alignment alignment;
};

NTF_DEFINE_TEMPLATE_CHECKER(bitmap_font_data);

template<glyph_char_type CharT, tex_depth_type DepthT>
struct font_load_t {};
template<typename CharT, typename DepthT>
constexpr font_load_t<CharT, DepthT> font_load;

template<typename Loader, typename CharT, typename DepthT>
concept checked_font_loader_type =
  requires(Loader& loader, font_load_t<CharT, DepthT> tag,
           const std::string& path, font_charset_view<CharT> charset, uvec2 glyph_size,
           decltype(loader.load(tag,path,charset,glyph_size)) data ) {
    { loader.load(tag, path, charset, glyph_size) } -> expected_with_error<asset_error>;
    { loader.glyphs(*data) } -> std::convertible_to<
      unique_array<glyph_data<CharT>, typename Loader::template deleter<glyph_data<CharT>>>
    >;
    { loader.texels(*data) } -> std::convertible_to<
      unique_array<DepthT, typename Loader::template deleter<DepthT>>
    >;
    { loader.format(*data) } -> std::convertible_to<r_texture_format>;
    { loader.alignment(*data) } -> std::convertible_to<r_image_alignment>;
  };

template<typename Loader, typename CharT, typename DepthT>
concept unchecked_font_loader_type = 
  requires(Loader& loader, font_load_t<CharT, DepthT> tag,
           const std::string& path, font_charset_view<CharT> charset, uvec2 glyph_size,
           decltype(loader.load(tag,path,charset,glyph_size)) data ) {
    { loader.load(tag, path, charset, glyph_size) } -> not_void;
    { loader.glyphs(data) } -> std::convertible_to<
      unique_array<glyph_data<CharT>, typename Loader::template deleter<glyph_data<CharT>>>
    >;
    { loader.texels(data) } -> std::convertible_to<
      unique_array<DepthT, typename Loader::template deleter<DepthT>>
    >;
    { loader.format(data) } -> std::convertible_to<r_texture_format>;
    { loader.alignment(data) } -> std::convertible_to<r_image_alignment>;
  };

template<typename Loader, typename CharT, typename DepthT>
concept font_loader_type =
  unchecked_font_loader_type<Loader, CharT, DepthT> ||
  checked_font_loader_type<Loader, CharT, DepthT>;

class ft2_bitmap_loader {
public:
  template<typename T>
  using deleter = allocator_delete<T, std::allocator<T>>;

  template<typename DepthT>
  using texel_array = unique_array<DepthT, deleter<DepthT>>;

  template<typename CharT>
  using glyph_array = unique_array<glyph_data<CharT>, deleter<glyph_data<CharT>>>;

  template<typename CharT, typename DepthT>
  struct data_t {
    texel_array<DepthT> texels;
    glyph_array<CharT> glyphs;
    r_texture_format format;
    r_image_alignment alignment;
  };

public:
  ft2_bitmap_loader() noexcept;
  ~ft2_bitmap_loader() noexcept;

public:
  asset_expected<data_t<char, uint8>> load(font_load_t<char, uint8>,
                                           const std::string& path,
                                           font_charset_view<char> charset, uvec2 glyph_size);

public:
  template<typename CharT, typename DepthT>
  texel_array<DepthT>&& texels(data_t<CharT, DepthT>& data) {
    return std::move(data.texels);
  }
  template<typename CharT, typename DepthT>
  texel_array<DepthT>&& texels(data_t<CharT, DepthT>&& data) {
    return std::move(data.texels);
  }

  template<typename CharT, typename DepthT>
  glyph_array<CharT>&& glyphs(data_t<CharT, DepthT>& data) {
    return std::move(data.glyphs);
  }
  template<typename CharT, typename DepthT>
  glyph_array<CharT>&& glyphs(data_t<CharT, DepthT>&& data) {
    return std::move(data.glyphs);
  }

  template<typename CharT, typename DepthT>
  r_texture_format format(data_t<CharT, DepthT>& data) {
    return data.format;
  }
  template<typename CharT, typename DepthT>
  r_texture_format format(data_t<CharT, DepthT>&& data) {
    return data.format;
  }

  template<typename CharT, typename DepthT>
  r_image_alignment alignment(data_t<CharT, DepthT>& data) {
    return data.alignment;
  }
  template<typename CharT, typename DepthT>
  r_image_alignment alignment(data_t<CharT, DepthT>&& data) {
    return data.alignment;
  }

private:
  void* _ft2_lib;
};
static_assert(checked_font_loader_type<ft2_bitmap_loader, char, uint8>);

} // namespace ntf
