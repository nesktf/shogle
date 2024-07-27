#pragma once

#include <shogle/core/types.hpp>
#include <shogle/render/render.hpp>

#include <map>

#include <fmt/format.h>

namespace ntf::shogle {

struct font_glyph {
  ivec2 size;
  ivec2 bearing;
  unsigned long advance;
};

class font {
public:
  font() = default;
  font(std::map<uint8_t, std::pair<uint8_t*, font_glyph>> chara);

public:
  size_t glyph_count() const { return _glyph_tex.size(); }

public:
  ~font();
  font(font&&) = default;
  font(const font&) = delete;
  font& operator=(font&&) = default;
  font& operator=(const font&) = delete;

private:
  std::map<uint8_t, std::pair<GLuint, font_glyph>> _glyph_tex;

private:
  friend void render_draw_text(const font& font, vec2 pos, float scale, std::string_view text);
};

void render_draw_text(const font& font, vec2 pos, float scale, std::string_view text);

template<typename... Args>
void render_draw_text(const font& f, vec2 p, float s, fmt::format_string<Args...> fmt, Args&&... args) {
  std::string str = fmt::format(fmt, std::forward<Args>(args)...);
  render_draw_text(f, p, s, str);
}


} // namespace ntf::shogle
