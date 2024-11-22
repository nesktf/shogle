#pragma once

#include "./assets.hpp"
#include "../render/render.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace ntf {

struct basic_font_data {
  glyph_map glyphs;
};

template<typename Font>
class freetype2_loader {
public:
  using resource_type = Font;
  using data_type = basic_font_data;

private:
  using font_type = Font;

public:
  // TODO: Pass custom allocator
  bool resource_load(std::string_view path, uint8_t count = 128, uint pixel_size = 48);
  void resource_unload(bool overwrite);

  std::optional<resource_type> make_resource() const;

public:
  const data_type& data() const { return _data; }
  data_type& data() { return _data; }

private:
  data_type _data;
};

template<typename T, typename Loader = freetype2_loader<T>>
using font_data = resource_data<T, Loader>;

} // namespace ntf

#ifndef SHOGLE_ASSETS_FONT_INL
#include "./font.inl"
#endif
