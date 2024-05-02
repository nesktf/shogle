#pragma once

#include <shogle/scene/entity.hpp>
#include <shogle/scene/camera.hpp>
#include <shogle/scene/tasker.hpp>

#include <shogle/render/model.hpp>

namespace ntf {

class model : public entity3D {
public:
  model(render::model* model, render::shader* shader, camera3D& cam);

public:
  virtual void draw(void) override;

public:
  inline void set_shader(render::shader* shader) { _shader = shader; }
  inline void set_model(render::model* model) { _model = model; }
  inline void set_cam(camera3D& cam) { _cam = cam; };

public:
  bool use_screen_space {false};

protected:
  virtual void update_shader(void);

private:
  render::model* _model;
  render::shader* _shader;
  camera3D& _cam;
};

struct dynamic_model : public tasker<model, dynamic_model> { 
  template<typename... Args>
  dynamic_model(Args&&... args) : 
    tasker(std::forward<Args>(args)...) {}
};

} // namespace ntf
