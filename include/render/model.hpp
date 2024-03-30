#pragma once

#include "resource/model.hpp"
#include "resource/shader.hpp"

#include "types.hpp"

namespace ntf::shogle::render {

class Model {
public:
  Model(cref<res::Model> _model, cref<res::Shader> _shader) :
    model_ref(_model),
    shader_ref(_shader) {}

public:
  void draw(void);
  static glm::mat4 model_transform(TransformData transform);

public:
  cref<res::Model> model_ref;
  cref<res::Shader> shader_ref;
  glm::mat4 model_m {1.0f};
};

} // namespace ntf::shogle

namespace ntf::shogle {

template<typename T>
class GameObject;

using ModelObj = GameObject<render::Model>;

} // namespace ntf::shogle
