#pragma once

#include "render/renderer.hpp"

#include "res/model.hpp"

namespace ntf {

struct ModelRenderer : public Renderer {
  ModelRenderer(Model3D* _model, Shader* _shader) :
    Renderer(_shader),
    model(_model) {}

  void draw(glm::mat4 model_m) override;

  Model3D* model;
};

} // namespace ntf
