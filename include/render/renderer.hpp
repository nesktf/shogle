#pragma once

#include "res/shader.hpp"

namespace ntf {

struct TransformData;

struct Renderer {
  Renderer(Shader* _shader) :
    shader(_shader) {}
  virtual ~Renderer() = default;

  virtual void draw(glm::mat4 model_m) = 0;

  Shader* shader;
};

} // namespace ntf
