#pragma once

#include <shogle/scene/entity.hpp>
#include <shogle/scene/camera.hpp>

#include <shogle/render/model.hpp>

#include <shogle/core/tasker.hpp>

namespace ntf {

class Model : public Entity3D {
public:
  Model(render::model* model, render::shader* shader, Camera3D* cam = &Camera3D::default_cam);

public:
  virtual void update(float dt) override;
  virtual void update_shader(void);

public:
  inline void draw() { _model->draw(*_shader); }

  inline void set_shader(render::shader* shader) { _shader = shader; }
  inline void set_model(render::model* model) { _model = model; }
  inline void set_cam(Camera3D* cam) { _cam = cam; };

public:
  bool use_screen_space {false};
  bool draw_on_update {false};

private:
  render::model* _model;
  render::shader* _shader;
  Camera3D* _cam;
};

struct ModelDynamic : public Tasker<Model> { 
  template<typename... Args>
  ModelDynamic(Args&&... args) : 
    Tasker<Model>(std::forward<Args>(args)...) {}
};

} // namespace ntf
