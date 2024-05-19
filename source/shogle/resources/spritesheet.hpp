#pragma once

#include <shogle/resources/texture.hpp>
#include <unordered_map>
#include <vector>

namespace ntf::shogle::resources {

struct sprite {
public:
  sprite(size_t count) : const_offset(count) {}

public:
  size_t count() const { return const_offset.size(); }
  vec2 get_const_offset(size_t i) const { return const_offset[i%count()]; }

public:
  vec2 corrected_scale;
  vec2 linear_offset;
  std::vector<vec2> const_offset;
  wptr<const texture2d> texture;
};

struct spritesheet_data {
public:
  struct sprite_data {
    std::string name;
    size_t count;
    size_t x, y;
    size_t x0, y0;
    size_t dx, dy;
    size_t cols, rows;
  };

public:
  spritesheet_data(std::string _path);

public:
  std::string path;
  texture2d_data texture;
  std::vector<sprite_data> sprites;
};

class spritesheet {
public:
  using data_t = spritesheet_data;

public:
  spritesheet(std::string path);
  spritesheet(data_t data);

public:
  GLuint tex_id() const { return _texture.tex_id(); }
  sprite& operator[](std::string name) { return _sprites.at(name); }

  void set_filter(gl::texture::filter filter) { _texture.set_filter(filter); }

  std::string path() const { return _path; }

private:
  std::string _path;
  texture2d _texture;
  std::unordered_map<std::string, sprite> _sprites;
};

} // namespace ntf::shogle::resources
