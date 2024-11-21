#include "./font.hpp"

namespace ntf {
void gl_font::load(const glyph_map& chara) {
  NTF_ASSERT(_vao == 0 && _vbo == 0, "gl_font already initialized");
  NTF_ASSERT(chara.size() > 0, "Can't load 0 fonts in gl_font!");

  glGenVertexArrays(1, &_vao);
  glGenBuffers(1, &_vbo);

  glBindVertexArray(_vao);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*6*4, nullptr, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);

  glBindVertexArray(0);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  GLuint tex;
  for (auto& [c, pair] : chara) {
    auto& [buf, ch] = pair;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, ch.size.x, ch.size.y, 0, GL_RED, GL_UNSIGNED_BYTE, buf);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    _atlas.insert(std::make_pair(c, std::make_pair(tex, ch)));
  }
  glBindTexture(GL_TEXTURE_2D, 0);

  SHOGLE_LOG(verbose, "[ntf::gl_font] Font loaded (ids: {}-{}, glyphs: {})",
             tex-_atlas.size()+1, tex, _atlas.size());
}

void gl_font::unload() {
  if (valid()) {
    [[maybe_unused]] GLuint last_tex;
    for (auto& [_, pair] : _atlas) {
      auto& [tex, ch] = pair;
      last_tex = tex;
      glDeleteTextures(1, &tex);
    }
    SHOGLE_LOG(verbose, "[ntf::gl_font] Font destroyed (ids: {}-{})",
               last_tex-_atlas.size()+1, last_tex);
  }
}

ivec2 gl_font::text_size(std::string_view text) const {
  NTF_ASSERT(valid(), "Invalid gl_font");

  ivec2 dim {0, 0};

  for (auto c = text.cbegin(); c != text.cend(); ++c) {
    auto [tex, ch] = _atlas.at(*c);
    dim.x += (ch.advance >> 6);
    // TODO: do the same with y
  }

  return dim;
}

void gl_font::draw_text(vec2 pos, float scale, std::string_view text) const {
  NTF_ASSERT(valid(), "Invalid gl_font");

  glBindVertexArray(_vao);

  float x = pos.x, y = pos.y;
  std::string_view::const_iterator c;
  for (c = text.begin(); c != text.end(); ++c) {
    auto [tex, ch] = _atlas.at(*c);

    float xpos = x + ch.bearing.x*scale;
    float ypos = y - ch.bearing.y*scale;

    float w = ch.size.x*scale;
    float h = ch.size.y*scale;

    float vert[6][4] {
      { xpos,     ypos + h, 0.0f, 1.0f },
      { xpos,     ypos,     0.0f, 0.0f },
      { xpos + w, ypos,     1.0f, 0.0f },

      { xpos,     ypos + h, 0.0f, 1.0f },
      { xpos + w, ypos,     1.0f, 0.0f },
      { xpos + w, ypos + h, 1.0f, 1.0f }
    };
    glBindTexture(GL_TEXTURE_2D, tex);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vert), vert);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    x += (ch.advance >> 6)*scale;
  }
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

gl_font::~gl_font() noexcept { unload(); }

gl_font::gl_font(gl_font&& f) noexcept :
  _vao(std::move(f._vao)), _vbo(std::move(f._vbo)),
  _atlas(std::move(f._atlas)) { f._vao = 0; f._vbo = 0; }

auto gl_font::operator=(gl_font&& f) noexcept -> gl_font& {
  unload();

  _vao = std::move(f._vao);
  _vbo = std::move(f._vbo);
  _atlas = std::move(f._atlas);

  f._vao = 0;
  f._vbo = 0;

  return *this;
}

} // namespace ntf
