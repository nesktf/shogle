#pragma once

#include <shogle/core/types.hpp>
#include <shogle/render/render.hpp>

#include <map>

#include <fmt/format.h>

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
  friend void render_draw_text(const font& font, vec2 pos, float scale, std::string_view text);
};

void render_draw_text(const font& font, vec2 pos, float scale, std::string_view text);

template<typename... Args>
void render_draw_text(const font& f, vec2 p, float s, fmt::format_string<Args...> fmt, Args&&... args) {
  std::string str = fmt::format(fmt, std::forward<Args>(args)...);
  render_draw_text(f, p, s, str);
}


} // namespace ntf::shogle
