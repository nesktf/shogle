#pragma once

#include "render/drawable.hpp"

namespace ntf::shogle {

class GameObject {
public:
  virtual ~GameObject() = default;

public:
  virtual void update(float dt) = 0;

public:
  glm::vec3 pos, scale, rot;
  Drawable* obj;
};

} // namespace ntf::shogle
