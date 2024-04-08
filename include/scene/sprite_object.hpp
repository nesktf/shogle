#pragma once

#include "scene/scene_object.hpp"
#include "scene/task.hpp"

#include "render/sprite_renderer.hpp"

namespace ntf {

struct SpriteObj : public SceneObj {
  SpriteObj(Texture* tex, Shader* sha) :
    sprite(tex, sha) {}
  
  void update(float dt) override;
  void draw(void) override;

  SpriteRenderer sprite;

  TaskManager<SpriteObj> tasks;

  glm::vec2 pos {0.0f};
  glm::vec2 scale {1.0f};
  float rot {0.0f};
};

} // namespace ntf
