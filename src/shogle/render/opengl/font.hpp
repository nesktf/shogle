#pragma once

#include "./common.hpp"

namespace ntf {

class gl_font {
public:
  using renderer_type = gl_context;
  using font_atlas = std::map<uint8_t, std::pair<GLuint, font_glyph>>;

public:
  gl_font() = default;

  gl_font(font_atlas atlas) :
    _atlas(std::move(atlas)) {}

  gl_font(const glyph_map& chara)
    { load(chara); }

public:
  void load(const glyph_map& chara);
  void unload();

public:
  ivec2 text_size(std::string_view text) const;

  void draw_text(vec2 pos, float scale, std::string_view text) const;

  template<typename... Args>
  void draw_text(vec2 pos, float scale, fmt::format_string<Args...> fmt, Args&&... arg) const {
    std::string str = fmt::format(fmt, arg...);
    draw_text(pos, scale, str);
  }

public:
  size_t glyph_count() const { return _atlas.size(); }
  bool valid() const { return _vao != 0 && _vbo != 0 && _atlas.size() > 0; }

  explicit operator bool() const { return valid(); }

private:
  GLuint _vao{0}, _vbo{0};
  font_atlas _atlas;

public:
  NTF_DECLARE_MOVE_ONLY(gl_font);
};

} // namespace ntf
