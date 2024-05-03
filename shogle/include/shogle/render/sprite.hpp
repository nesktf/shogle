#pragma once

#include <shogle/render/shader.hpp>

namespace ntf::render {

class sprite {
public:
  using data_t = res::spritesheet_loader::sprite;
  using loader_t = res::texture_loader;

public:
  sprite(std::string path); // unique constructors
  sprite(loader_t loader);

  sprite(gl::texture* tex, data_t data); // spritesheet constructor
  sprite(gl::texture* tex, size_t w, size_t h); // fbo constructor
  
public:
  ~sprite();
  sprite(sprite&&) noexcept;
  sprite(const sprite&) = delete;
  sprite& operator=(sprite&&) noexcept;
  sprite& operator=(const sprite&) = delete;

public:
  void draw(shader& shader, size_t index = 0, bool inverted_draw = false) const;

public:
  inline size_t count(void) const { return _uniform_offset_const.size(); }
  inline float aspect(void) const { return _aspect; }

private:
  bool _unique {false};
  gl::texture* _tex;
  float _aspect;
  vec2 _uniform_offset_linear;
  std::vector<vec2> _uniform_offset_const;
};

class spritesheet { 
public:
  using loader_t = res::spritesheet_loader;

public:
  spritesheet(loader_t loader);
  
public:
  inline sprite* get_sprite(std::string name) { return &_sprites.at(name); }

private:
  uptr<gl::texture> _tex;
  std::unordered_map<std::string, sprite> _sprites;
};

} // namespace ntf::render
