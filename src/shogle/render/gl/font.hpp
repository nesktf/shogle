#pragma once

#include <shogle/render/gl.hpp>

namespace ntf {

class gl::font {
public:
  using renderer = gl;

public:
  font() = default;

  font(font_atlas atlas) :
    _atlas(std::move(atlas)) {}

  font(std::map<uint8_t, std::pair<uint8_t*, font_glyph>> chara);

public:
  ivec2 text_size(std::string_view text) const;

  void draw_text(vec2 pos, float scale, std::string_view text) const;

  template<typename... Args>
  void draw_text(vec2 pos, float scale, fmt::format_string<Args...> fmt, Args&&... args) const;

  void unload();

public:
  size_t glyph_count() const { return _atlas.size(); }

private:
  font_atlas _atlas;

public:
  NTF_DECLARE_MOVE_ONLY(font);
};

} // namespace ntf

#ifndef SHOGLE_RENDER_FONT_INL
#include <shogle/render/gl/font.inl>
#endif
