#pragma once

#include "render/drawable.hpp"
#include "resource/texture.hpp"

namespace ntf::shogle {

class Sprite : public Drawable {
public:
  Sprite(res::Texture* _texture, res::Shader* _shader) :
    Drawable(_shader),
    texture(_texture) {}

public:
  virtual void draw(void) override;

public:
  res::Texture* texture;
};

} // namespace ntf::shogle
