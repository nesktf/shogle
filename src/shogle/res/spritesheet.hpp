#pragma once

#include <shogle/render/texture.hpp>
#include <shogle/res/loader_data.hpp>

#include <unordered_map>
#include <vector>

namespace ntf::shogle {


class spritesheet;

struct sprite {
public:
  sprite(texture2d& tex, size_t count) :
    _texture(tex), _const_offset(count) {};

public:
  size_t count() const { return _const_offset.size(); }
  vec4 tex_offset(size_t i) const { return vec4{_linear_offset, _const_offset[i%count()]}; }
  vec2 corrected_scale() const { return _corrected_scale; }
  texture2d& tex() { return _texture; }

private:
  texture2d& _texture;
  vec2 _corrected_scale;
  vec2 _linear_offset;
  std::vector<vec2> _const_offset;

  friend class spritesheet;
};


class spritesheet {
public:
  spritesheet(spritesheet_loader loader);

  spritesheet(std::string_view path) :
    spritesheet(spritesheet_loader{path}) {}

public:
  sprite& operator[](std::string name) { return _sprites.at(name); }
  texture2d& tex() { return _texture; }

private:
  texture2d _texture;
  std::unordered_map<std::string, sprite> _sprites;
};

} // namespace ntf::shogle
