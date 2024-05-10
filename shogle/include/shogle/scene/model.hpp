#pragma once

#include <shogle/scene/entity.hpp>
#include <shogle/scene/camera.hpp>
#include <shogle/scene/tasker.hpp>

#include <shogle/render/model.hpp>
#include <shogle/res/global.hpp>

namespace ntf {

class model : public entity3d {
public:
  model(wptr<render::model> model);
  model(wptr<render::model> model, wptr<render::shader> shader);
  model(wptr<render::model> model, wptr<camera3d> cam);
  model(wptr<render::model> model, wptr<camera3d> cam, wptr<render::shader> shader);

public:
  virtual void draw(void) override;

public:
  inline void set_model(wptr<render::model> model) { _model = model; }
  inline void set_cam(wptr<camera3d> cam) { _cam = cam; };
  inline void set_shader(wptr<render::shader> shader) { _shader = shader; }

protected:
  virtual void update_shader(void);

private:
  wptr<render::model> _model;
  wptr<render::shader> _shader {res::def_model_sh.get()};
  wptr<camera3d> _cam {&res::def_cam3d};
};

struct dynamic_model : public tasker<model, dynamic_model> { 
  template<typename... Args>
  dynamic_model(Args&&... args) : 
    tasker(std::forward<Args>(args)...) {}
};

} // namespace ntf
