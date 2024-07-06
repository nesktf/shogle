#include <shogle/render/font.hpp>

#include <shogle/core/log.hpp>

namespace ntf::shogle {

font::font(std::map<uint8_t, std::pair<uint8_t*, character>> chara) {
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  for (auto& [c, pair] : chara) {
    auto& [buff, ch] = pair;

    tex_t tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, ch.size.x, ch.size.y, 0, GL_RED, GL_UNSIGNED_BYTE, buff);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    _chara.insert(std::make_pair(c, std::make_pair(tex, ch)));
  }
  glBindTexture(GL_TEXTURE_2D, 0);

  log::verbose("[shogle::font] Loaded {} glyphs", chara.size());
}

font::~font() {
  for (auto& [_, pair] : _chara) {
    auto& [tex, ch] = pair;
    glDeleteTextures(1, &tex);
  }
  log::verbose("[shogle::font] Font unloaded");
}

// void render_draw_text(const font& font, std::string_view text, vec2 pos, float scale, color4 color);
// defined in render.cpp

} // namespace ntf::shogle
