#pragma once

#include "res/texture.hpp"

namespace ntf {

class SpriteRenderer {
protected:
  using res_t = Texture;

  SpriteRenderer(res_t* tex, Shader* sha) :
    _texture(tex),
    _shader(sha) {}

public:
  void draw_sprite(void);

protected:
  res_t* _texture;
  Shader* _shader;
};

} // namespace ntf
