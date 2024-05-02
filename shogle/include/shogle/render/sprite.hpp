#pragma once

#include <shogle/render/shader.hpp>

namespace ntf::render {

class sprite {
public:
  using renderer = gl;
  using data_t = res::spritesheet_loader::sprite;

public:
  sprite(renderer::texture* tex, data_t data);
  sprite(renderer::texture* tex, size_t w, size_t h);

public:
  void draw(shader& shader, size_t index = 0, bool inverted_draw = false) const;

public:
  inline size_t count(void) const { return _uniform_offset_const.size(); }
  inline float aspect(void) const { return _aspect; }

private:
  float _aspect;
  renderer::texture* _tex;
  vec2 _uniform_offset_linear;
  std::vector<vec2> _uniform_offset_const;
};

class spritesheet { 
public:
  using renderer = gl;
  using loader_t = res::spritesheet_loader;

public:
  spritesheet(loader_t loader);
  
public:
  inline sprite* get(std::string name) { return &_sprites.at(name); }

private:
  uptr<renderer::texture> _tex;
  std::unordered_map<std::string, sprite> _sprites;
};

} // namespace ntf::render
