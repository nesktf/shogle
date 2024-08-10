#pragma once

#include <shogle/render/render.hpp>

#include <shogle/core/types.hpp>
#include <shogle/core/log.hpp>

#include <map>

#include <fmt/format.h>

namespace ntf {

struct font_glyph {
  ivec2 size;
  ivec2 bearing;
  unsigned long advance;
};

class font {
public:
  font() = default;
  inline font(std::map<uint8_t, std::pair<uint8_t*, font_glyph>> chara);

public:
  size_t glyph_count() const { return _glyph_tex.size(); }

public:
  inline ~font();
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

font::font(std::map<uint8_t, std::pair<uint8_t*, font_glyph>> chara) {
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  for (auto& [c, pair] : chara) {
    auto& [buff, ch] = pair;

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, ch.size.x, ch.size.y, 0, GL_RED, GL_UNSIGNED_BYTE, buff);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    _glyph_tex.insert(std::make_pair(c, std::make_pair(tex, ch)));
  }
  glBindTexture(GL_TEXTURE_2D, 0);

  log::verbose("[shogle::font] Loaded {} glyphs", chara.size());
}

font::~font() {
  if (glyph_count() != 0) { // May be empty
    for (auto& [_, pair] : _glyph_tex) {
      auto& [tex, ch] = pair;
      glDeleteTextures(1, &tex);
    }
    log::verbose("[shogle::font] Font unloaded");
  }
}

} // namespace ntf
