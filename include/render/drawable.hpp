#pragma once

#include "resource/shader.hpp"

namespace ntf::shogle {

class Drawable {
public:
  Drawable(res::Shader* shader) :
    shader(shader) {
    model_m = glm::mat4(1.0f);
  }
  virtual ~Drawable() = default;

public:
  virtual void draw(void) = 0;

public:
  glm::mat4 model_m;
  res::Shader* shader;
};

} // namespace ntf::shogle
