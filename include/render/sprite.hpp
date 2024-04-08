#pragma once

#include "render/base_renderer.hpp"

#include "res/texture.hpp"

namespace ntf {

class SpriteRenderer : public BaseRenderer {
public:
  SpriteRenderer(Texture* _texture, Shader* _shader) :
    BaseRenderer(_shader),
    texture(_texture) {}

public:
  void draw(glm::mat4 model_m) override;

public:
  static glm::mat4 default_modelgen(TransformData transform);

public:
  Texture* texture;
};

using SpriteObj = SceneObj<SpriteRenderer>;

} // namespace ntf
