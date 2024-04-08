#pragma once

#include "render/base_renderer.hpp"

#include "res/model.hpp"

namespace ntf {

class ModelRenderer : public BaseRenderer {
public:
  ModelRenderer(Model* _model, Shader* _shader) :
    BaseRenderer(_shader),
    model(_model) {}

public:
  void draw(glm::mat4 model_m) override;

public:
  static glm::mat4 default_modelgen(TransformData transform);

public:
  Model* model;
};

using ModelObj = SceneObj<ModelRenderer>;

} // namespace ntf
