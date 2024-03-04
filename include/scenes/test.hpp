#pragma once

#include "core/scene.hpp"
#include "core/sprite.hpp"
#include "core/model.hpp"

namespace ntf::shogle {

class TestScene : public Scene {
public:
  TestScene();
  ~TestScene();

  void update(float delta_time) override;
  void draw(void) override;

  Sprite* chiruno_sprite;
  Model* chiruno_fumo;
  Shader* shader;
  Texture* chiruno_tex;
};

}
