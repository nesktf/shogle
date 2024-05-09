#pragma once

#include <shogle/render/shader.hpp>

namespace ntf::render {

struct sprite {
public:
  using loader_t = res::texture_loader;
  using spritedata_t = res::spritesheet_loader::sprite;

public:
  sprite(std::string path); // unique constructors
  sprite(loader_t loader);

  sprite(gl::texture sheet_tex, spritedata_t data); // spritesheet constructor
  sprite(gl::texture fbo_tex, size_t w, size_t h); // fbo constructor

public:
  inline float aspect(void) const { return _aspect; }
  inline size_t count(void) const { return _const_off.size(); }
  inline vec4 uniform_offset(size_t i) const { return {_linear_off, _const_off[i % count()]}; }

public:
  gl::texture _tex;
  bool _unique;
  vec2 _linear_off;
  std::vector<vec2> _const_off;
  float _aspect;

public:
  ~sprite();
  sprite(sprite&&) = default;
  sprite(const sprite&) = delete;
  sprite& operator=(sprite&&) noexcept;
  sprite& operator=(const sprite&) = delete;
};

struct spritesheet { 
public:
  using loader_t = res::spritesheet_loader;

public:
  spritesheet(std::string path);
  spritesheet(loader_t loader);
  
public:
  inline sprite* operator[](std::string name) { return &_sprites.at(name); }
  inline sprite* get_sprite(std::string name) { return &_sprites.at(name); }

public:
  gl::texture _tex;
  std::unordered_map<std::string, sprite> _sprites;

public:
  ~spritesheet();
  spritesheet(spritesheet&&) = default;
  spritesheet(const spritesheet&) = delete;
  spritesheet& operator=(spritesheet&&) noexcept;
  spritesheet& operator=(const spritesheet&) = delete;
};

void draw_sprite(sprite& sprite, shader& shader, size_t index = 0, bool inverted = false);

} // namespace ntf::render
