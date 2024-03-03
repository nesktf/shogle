#pragma once

#include "core/game_object.hpp"
#include <unordered_map>

namespace ntf::shogle {

class Scene {
public:
  Scene();
  virtual ~Scene();

  virtual void update(float delta_time) = 0;
  virtual void draw(const Renderer& renderer);

protected:
  std::unordered_map<unsigned int, GameObject*> objs;
};

}
