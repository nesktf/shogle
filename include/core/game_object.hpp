#pragma once

#include "core/texture.hpp"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace ntf::shogle {

class GameObject {
public:
  virtual ~GameObject() {}

  virtual void update(float delta_time) = 0;
  virtual void draw(Shader& shader) = 0;

public:
  glm::mat4 model_m;
};

}
