#pragma once

#include "scene/scene_object.hpp"

#include "render/model_renderer.hpp"

namespace ntf {

class Model : public SceneObj<Model, ModelRenderer> {
public:
  Model(ModelRes* mod, Shader* sha);

protected:
  virtual void shader_update(Shader* shader, glm::mat4 model_m) override;
  virtual glm::mat4 model_m_gen(void) override;

public:
  glm::vec3 pos {0.0f};
  glm::vec3 scale {1.0f};
  glm::vec3 rot {1.0f};
};

} // namespace ntf
