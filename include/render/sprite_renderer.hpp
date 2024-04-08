#pragma once

#include "render/renderer.hpp"

#include "res/texture.hpp"

namespace ntf {

struct SpriteRenderer : public Renderer {
  SpriteRenderer(Texture* _texture, Shader* _shader) :
    Renderer(_shader),
    texture(_texture) {}

  void draw(glm::mat4 model_m) override;

  Texture* texture;
};

} // namespace ntf
