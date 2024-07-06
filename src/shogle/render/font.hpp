#pragma once

#include <shogle/core/types.hpp>
#include <shogle/render/render.hpp>

#include <map>

namespace ntf::shogle {

class font {
public:
  using tex_t = GLuint;
  struct character {
    ivec2 size;
    ivec2 bearing;
    unsigned long advance;
  };

public:
  font(std::map<uint8_t, std::pair<uint8_t*, character>> chara);
  ~font();

private:
  std::map<uint8_t, std::pair<tex_t, character>> _chara;

private:
  friend void render_draw_text(const font& font, std::string_view text, vec2 pos, float scale);
};

void render_draw_text(const font& font, std::string_view text, vec2 pos, float scale);

} // namespace ntf::shogle
