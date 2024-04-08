#pragma once

#include "res/shader.hpp"

namespace ntf {

struct TransformData;

struct BaseRenderer {
protected:
  BaseRenderer(Shader* _shader) :
    shader(_shader) {}

public:
  virtual ~BaseRenderer() = default;
  virtual void draw(glm::mat4 model_m) = 0;

  Shader* shader;
};

template<typename TRenderer>
class SceneObj;

} // namespace ntf
