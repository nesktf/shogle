#pragma once

#include "scene/scene_object.hpp"

#include "render/sprite_renderer.hpp"

namespace ntf {

struct Sprite : public SceneObj<Sprite> {
  Sprite(Texture* tex, Shader* sha) :
    sprite(tex, sha) {}
  
  void update(float dt) override;
  void draw(void) override;

  SpriteRenderer sprite;

  glm::vec2 pos {0.0f};
  glm::vec2 scale {1.0f};
  float rot {0.0f};
  unsigned int layer {0};
};

using SpriteTaskFun = TaskFun<Sprite>;

} // namespace ntf
