#pragma once

#include "render/drawable.hpp"
#include "resource/texture.hpp"

namespace ntf::shogle {

class Sprite : public Drawable {
public:
  Sprite(res::Texture* texture, res::Shader* shader) :
    Drawable(shader),
    texture(texture) {}

public:
  virtual void draw(void) override;

public:
  res::Texture* texture;
};

} // namespace ntf::shogle
