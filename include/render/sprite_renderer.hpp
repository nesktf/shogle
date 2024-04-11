#pragma once

#include "res/texture.hpp"

namespace ntf {

class SpriteRenderer {
protected:
  using res_t = Texture;

  SpriteRenderer(res_t* tex, Shader* sha) :
    texture(tex),
    _shader(sha) {}

public:
  void draw(void);

protected:
  res_t* texture;
  Shader* _shader;
};

} // namespace ntf
