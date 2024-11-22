#include "./font.hpp"

namespace ntf {

gl_font::gl_font(const glyph_map& chara, tex_filter filter) {
  _load(chara, filter);
}

gl_font& gl_font::load(const glyph_map& chara, tex_filter filter) & {
  _load(chara, filter);
  return *this;
}

gl_font&& gl_font::load(const glyph_map& chara, tex_filter filter) && {
  _load(chara, filter);
  return std::move(*this);
}

gl_font& gl_font::fliter(tex_filter filter) & {
  _set_filter(filter);
  return *this;
}

void gl_font::unload() {
  if (!valid()) {
    return;
  }

  [[maybe_unused]] GLuint last_tex;
  for (auto& [_, pair] : _atlas) {
    auto& [tex, ch] = pair;
    last_tex = tex;
    glDeleteTextures(1, &tex);
  }

  SHOGLE_LOG(verbose, "[ntf::gl_font] Font destroyed (vao: {}, ids: {}-{})",
             _vao, last_tex-_atlas.size()+1, last_tex);

  glDeleteVertexArrays(1, &_vao);
  glDeleteBuffers(1, &_vbo);
}

ivec2 gl_font::text_size(std::string_view text) const {
  NTF_ASSERT(valid(), "Invalid gl_font");

  ivec2 dim {0, 0};
  for (const auto c : text) {
    GLuint tex;
    font_glyph glyph;

    if (_atlas.find(c) != _atlas.end()) {
      std::tie(tex, glyph) = _atlas.at(c);
    } else {
      std::tie(tex, glyph) = _atlas.at('?');
    }

    dim.x += (glyph.advance >> 6);
    dim.y = std::max(glyph.size.y, dim.y);
  }

  return dim;
}


void gl_font::_load(const glyph_map& chara, tex_filter filter) {
  if (!_vao) {
    GLuint vao{}, vbo{};
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*6*4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);

    glBindVertexArray(0);

    _vao = vao;
    _vbo = vbo;
  }

  GLint prev_align{};
  glGetIntegerv(GL_UNPACK_ALIGNMENT, &prev_align);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  GLuint tex{};
  font_atlas atlas;
  const auto glfilter = enumtogl(filter);
  for (auto& [c, pair] : chara) {
    auto& [buf, glyph] = pair;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, glyph.size.x, glyph.size.y, 0, GL_RED, 
                 GL_UNSIGNED_BYTE, buf);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glfilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glfilter);

    atlas.insert(std::make_pair(c, std::make_pair(tex, glyph)));
  }
  glBindTexture(GL_TEXTURE_2D, 0);

  glPixelStorei(GL_UNPACK_ALIGNMENT, prev_align);

  if (_atlas.size() > 0) {
    SHOGLE_LOG(warning, "[ntf::gl_font] Font atlas overwritten (vao: {}, glyphs: {} -> {})",
               _vao, _atlas.size(), atlas.size());
  }

  SHOGLE_LOG(verbose, "[ntf::gl_font] Font loaded (vao: {}, ids: {}-{}, glyps: {})",
             _vao, tex-atlas.size()+1, tex, atlas.size());
  _atlas = std::move(atlas);
}

void gl_font::_set_filter(tex_filter filter) {
  NTF_ASSERT(valid());
  const auto glfilter = enumtogl(filter);
  for (auto& [ch, pair] : _atlas) {
    auto tex = pair.first;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glfilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glfilter);
  }
  glBindTexture(GL_TEXTURE_2D, 0);
}

void gl_font::_reset() {
  _vao = 0;
  _vbo = 0;
  _atlas.clear();
}


gl_font::~gl_font() noexcept { unload(); }

gl_font::gl_font(gl_font&& f) noexcept :
  _vao(std::move(f._vao)), _vbo(std::move(f._vbo)),
  _atlas(std::move(f._atlas)) { f._reset(); }

auto gl_font::operator=(gl_font&& f) noexcept -> gl_font& {
  unload();

  _vao = std::move(f._vao);
  _vbo = std::move(f._vbo);
  _atlas = std::move(f._atlas);

  f._reset();

  return *this;
}

} // namespace ntf
