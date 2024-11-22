#pragma once

#include "./common.hpp"

namespace ntf {

class gl_font {
public:
  using context_type = gl_context;
  using font_atlas = std::map<uint8_t, std::pair<GLuint, font_glyph>>;

public:
  gl_font() = default;

  gl_font(const glyph_map& chara, tex_filter filter = tex_filter::linear);

public:
  gl_font& load(const glyph_map& chara, tex_filter filter = tex_filter::linear) &;
  gl_font&& load(const glyph_map& chara, tex_filter filter = tex_filter::linear) &&;

  gl_font& fliter(tex_filter filter) &;

  void unload();

public:
  GLuint vao() const { return _vao; }
  GLuint vbo() const { return _vbo; }
  const font_atlas& atlas() const { return _atlas; }

  ivec2 text_size(std::string_view text) const;

  std::size_t glyphs() const { return _atlas.size(); }
  bool valid() const { return _vao != 0 && _vbo != 0; }
  bool empty() const { return _atlas.size() == 0; }

  explicit operator bool() const { return valid(); }

private:
  GLuint _vao{0}, _vbo{0};
  font_atlas _atlas;

private:
  void _load(const glyph_map& chara, tex_filter filter);
  void _set_filter(tex_filter filter);
  void _reset();

public:
  NTF_DECLARE_MOVE_ONLY(gl_font);
};

} // namespace ntf
