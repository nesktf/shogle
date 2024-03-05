#pragma once

#include "core/texture.hpp"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace ntf::shogle {

class GameObject {
public:
  virtual ~GameObject() = default;

public:
  virtual void update(float dt) = 0;
  virtual void draw(Shader& shader) const = 0;

public:
  glm::mat4 model_m;
};

}
