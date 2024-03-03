#pragma once

#include "core/game_object.hpp"
#include <unordered_map>

namespace ntf::shogle {

typedef std::pair<std::reference_wrapper<Shader>, GameObject*> ObjPair;

class Scene {
public:
  virtual ~Scene();

  virtual void update(float delta_time) = 0;
  virtual void draw(void) = 0;

};

}
