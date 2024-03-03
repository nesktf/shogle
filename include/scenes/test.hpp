#pragma once

#include "core/scene.hpp"

namespace ntf::shogle {

class TestScene : public Scene {
public:
  TestScene();
  void update(float delta_time) override;
  void draw(void) override;
};

}
