#pragma once

namespace ntf::render {

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



} // namespace ntf::render
