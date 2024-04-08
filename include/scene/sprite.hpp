#pragma once

#include "scene/scene_object.hpp"
#include "scene/task.hpp"

#include "render/sprite_renderer.hpp"

namespace ntf {

struct Sprite : public SceneObj {
  Sprite(Texture* tex, Shader* sha) :
    sprite(tex, sha) {}
  
  void update(float dt) override;
  void draw(void) override;

  SpriteRenderer sprite;

  TaskManager<Sprite> tasks;

  glm::vec2 pos {0.0f};
  glm::vec2 scale {1.0f};
  float rot {0.0f};
};

} // namespace ntf
