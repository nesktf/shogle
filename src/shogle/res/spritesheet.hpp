#pragma once

#include <shogle/render/texture.hpp>
#include <shogle/res/texture.hpp>

#include <unordered_map>
#include <vector>

namespace ntf::shogle {

struct sprite_data {
  std::string name;
  size_t count;
  size_t x, y;
  size_t x0, y0;
  size_t dx, dy;
  size_t cols, rows;
};

struct spritesheet_data {
public:
  spritesheet_data(std::string_view path_, tex_filter filter, tex_wrap wrap);

public:
  texture2d_data texture;
  std::vector<sprite_data> sprites;
};


class sprite {
public:
  sprite(texture2d& tex, vec2 scale, vec2 lin_off, std::vector<vec2> con_off);

public:
  size_t count() const { return _const_offset.size(); }
  vec4 tex_offset(size_t i) const { return vec4{_linear_offset, _const_offset[i%count()]}; }
  vec2 corrected_scale() const { return _corrected_scale; }
  const texture2d& tex() const { return *_texture; }

private:
  texture2d* _texture;
  vec2 _corrected_scale;
  vec2 _linear_offset;
  std::vector<vec2> _const_offset;
  friend class spritesheet;
};

class spritesheet {
public:
  spritesheet(texture2d_data texture, std::vector<sprite_data> sprites);
  spritesheet(spritesheet&&) noexcept;

public:
  sprite& operator[](std::string_view name);
  const texture2d& tex() const { return _texture; }

private:
  texture2d _texture;
  std::vector<std::pair<std::string, sprite>> _sprites;
};


spritesheet load_spritesheet(std::string_view path, tex_filter filter, tex_wrap wrap);
spritesheet load_spritesheet(spritesheet_data data);

} // namespace ntf::shogle
