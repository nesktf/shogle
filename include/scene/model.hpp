#pragma once

#include "scene/scene_object.hpp"

#include "render/model_renderer.hpp"

namespace ntf {

struct Model : public SceneObj<Model> {
  Model(ModelRes* mod, Shader* sha) :
    model(mod, sha) {} 

  void update(float dt) override;
  void draw(void) override;

  ModelRenderer model;

  glm::vec3 pos {0.0f};
  glm::vec3 scale {1.0f};
  glm::vec3 rot {1.0f};
};

using ModelTaskFun = TaskFun<Model>;

} // namespace ntf
