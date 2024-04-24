#pragma once

#include <shogle/scene/scene_object.hpp>
#include <shogle/scene/camera3d.hpp>

#include <shogle/core/renderer.hpp>

namespace ntf {

class ModelImpl : public ModelRenderer, public SceneObj {
protected:
  ModelImpl(const ModelRes* model, const Shader* shader);

public:
  virtual void update(float dt) override;
  void udraw(float dt) { update(dt); draw(); }

protected:
  mat4 _gen_model(void) override;
  virtual void _shader_update(void);

public:
  Camera3D* cam;

  vec3 pos {0.0f}, scale {1.0f}, rot {1.0f};

  bool use_screen_space {false};
};

struct Model : public TaskedObj<Model, ModelImpl> {
  Model(const ModelRes* model, const Shader* shader) :
    TaskedObj(model, shader) {}
};

} // namespace ntf
