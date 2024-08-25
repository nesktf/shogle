#define SHOGLE_RENDER_FONT_INL
#include <shogle/render/gl/font.hpp>
#undef SHOGLE_RENDER_FONT_INL

namespace ntf {

inline gl::font::font(std::map<uint8_t, std::pair<uint8_t*, font_glyph>> chara) {
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  GLuint tex;
  for (auto& [c, pair] : chara) {
    auto& [buff, ch] = pair;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, ch.size.x, ch.size.y, 0, GL_RED, GL_UNSIGNED_BYTE, buff);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    _atlas.insert(std::make_pair(c, std::make_pair(tex, ch)));
  }
  glBindTexture(GL_TEXTURE_2D, 0);

  SHOGLE_INTERNAL_LOG_FMT(verbose,
    "[SHOGLE][ntf::gl::font] Loaded (ids: {}-{}, glyphs: {})", tex-_atlas.size()+1, tex, _atlas.size());
}

inline gl::font::~font() noexcept {
#ifdef SHOGLE_GL_RAII_UNLOAD
  unload();
#endif
}

inline gl::font::font(font&& f) noexcept :
  _atlas(std::move(f._atlas)) {}

inline auto gl::font::operator=(font&& f) noexcept -> font& {
  unload();

  _atlas = std::move(f._atlas);

  return *this;
}

inline void gl::font::unload() {
  if (glyph_count() != 0) { // May be empty
    [[maybe_unused]] GLuint last_tex;
    for (auto& [_, pair] : _atlas) {
      auto& [tex, ch] = pair;
      last_tex = tex;
      glDeleteTextures(1, &tex);
    }
    SHOGLE_INTERNAL_LOG_FMT(verbose,
                            "[SHOGLE][ntf::gl::font] Unloaded (ids: {}-{})", last_tex-_atlas.size()+1, last_tex);
  }
}

inline ivec2 gl::font::text_size(std::string_view text) const {
  ivec2 dim {0, 0};

  for (auto c = text.cbegin(); c != text.cend(); ++c) {
    auto [tex, ch] = _atlas.at(*c);
    dim.x += (ch.advance >> 6);
    // TODO: do the same with y
  }

  return dim;
}

inline void gl::font::draw_text(vec2 pos, float scale, std::string_view text) const {
  renderer::draw_text(_atlas, pos, scale, text);
}

template<typename... Args>
void gl::font::draw_text(vec2 pos, float scale, fmt::format_string<Args...> fmt, Args&&... args) const {
  std::string str = fmt::format(fmt, std::forward<Args>(args)...);
  draw_text(pos, scale, str);
}

} // namespace ntf
