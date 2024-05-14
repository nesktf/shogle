#pragma once

#include <shogle/core/types.hpp>
#include <shogle/render/drawable.hpp>
#include <shogle/render/texture.hpp>

namespace ntf::render {

class shader;

extern uptr<gl::mesh> def_quad2d;
extern uptr<render::shader> def_sprite_sh;

class sprite : public drawable {
public:
  virtual void draw(shogle_state& state) override;
  
protected:
  wptr<gl::mesh> _quad {def_quad2d.get()};
  wptr<render::shader> _shader {def_sprite_sh.get()};
  wptr<render::texture> _texture;

  vec2 _pos{0.0f};
  vec2 _scale{1.0f};
  float _rot{0.0f};
};


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

void draw_sprite(sprite& sprite, shader& shader, size_t index = 0, bool inverted = false);

} // namespace ntf::render
