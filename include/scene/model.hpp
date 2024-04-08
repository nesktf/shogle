#pragma once

#include "scene/scene_object.hpp"
#include "scene/task.hpp"

#include "render/model_renderer.hpp"

namespace ntf {

struct Model : public SceneObj {
  Model(Model3D* mod, Shader* sha) :
    model(mod, sha) {} 

  void update(float dt) override;
  void draw(void) override;

  ModelRenderer model;

  TaskManager<Model> tasks;

  glm::vec3 pos {0.0f};
  glm::vec3 scale {1.0f};
  glm::vec3 rot {1.0f};
};

} // namespace ntf
