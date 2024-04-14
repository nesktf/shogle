#pragma once

#include "scene/scene_object.hpp"
#include "scene/camera3d.hpp"

#include "render/model_renderer.hpp"

namespace ntf {

class ModelImpl : public ModelRenderer, public SceneObj {
protected:
  ModelImpl(ModelRes* model, Shader* shader);

public:
  virtual void update(float dt) override;
  void draw(void) override { draw_model(); }

protected:
  mat4 _gen_model(void) override;
  virtual void _shader_update(void);

public:
  Camera3D* cam;

  vec3 pos {0.0f}, scale {1.0f}, rot {1.0f};
};

struct Model : public TaskedObj<Model, ModelImpl> {
  Model(ModelRes* model, Shader* shader) :
    TaskedObj(model, shader) {}
};

} // namespace ntf
