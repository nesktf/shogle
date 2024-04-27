#pragma once

#include <shogle/render/backends/gl.hpp>

namespace ntf::render {

class shader;

class sprite {
public:
  using renderer = gl;
  using data_t = res::spritesheet_loader::sprite;

public:
  sprite(const renderer::texture& tex, data_t data);

public:
  void draw(shader& shader, size_t index = 0) const;

public:
  inline size_t count(void) const { return _uniform_offset_const.size(); }

private:
  const renderer::texture& _tex;
  vec2 _uniform_offset_linear;
  std::vector<vec2> _uniform_offset_const;
};

class spritesheet { 
public:
  using renderer = gl;
  using loader_t = res::spritesheet_loader;

public:
  spritesheet(loader_t loader);

private:
  renderer::texture _tex;
  std::unordered_map<std::string, sprite> _sprites;
};

} // namespace ntf::render
