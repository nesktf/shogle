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

  glm::vec3 pos_v, scale_v, rot_v;
  glm::mat4 model_m;
};

}
