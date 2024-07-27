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
  spritesheet_data(std::string_view path_);

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

private:
  friend class spritesheet;
};

class spritesheet {
public:
  spritesheet() = default;
  spritesheet(texture2d_data tex_data, std::vector<sprite_data> sprites, tex_filter filter, tex_wrap wrap);

public:
  const sprite& operator[](std::string_view name) const { return _sprites.at(name.data()); }
  const sprite& at(std::string_view name) const { return _sprites.at(name.data()); }
  const texture2d& tex() const { return _texture; }
  size_t size() const { return _sprites.size(); }

public:
  ~spritesheet() = default;
  spritesheet(spritesheet&&) noexcept;
  spritesheet(const spritesheet&) = delete;
  spritesheet& operator=(spritesheet&&) noexcept;
  spritesheet& operator=(const spritesheet&) = delete;

private:
  texture2d _texture;
  std::unordered_map<std::string, sprite> _sprites;
};


inline spritesheet load_spritesheet(std::string_view path, tex_filter filter, tex_wrap wrap) {
  auto data = spritesheet_data{path};
  return spritesheet{std::move(data.texture), std::move(data.sprites), filter, wrap};
}

} // namespace ntf::shogle
