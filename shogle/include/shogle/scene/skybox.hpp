#pragma once

#include <shogle/scene/camera.hpp>

#include <shogle/render/model.hpp>
#include <shogle/res/global.hpp>

namespace ntf {

class skybox : public scene::drawable {
public:
  skybox(wptr<render::cubemap> cubemap);
  skybox(wptr<render::cubemap> cubemap, wptr<render::shader> shader);
  skybox(wptr<render::cubemap> cubemap, wptr<camera3d> cam);
  skybox(wptr<render::cubemap> cubemap, wptr<camera3d> cam, wptr<render::shader> shader);

public:
  void update(float) final {}; // depends on camera for updates
  virtual void draw(void) override;

public:
  void set_cubemap(wptr<render::cubemap> cubemap) { _cubemap = cubemap; }
  void set_cam(wptr<camera3d> cam) { _cam = cam; }
  void set_shader(wptr<render::shader> shader) { _shader = shader; }

protected:
  virtual void update_shader(void);

private:
  wptr<render::cubemap> _cubemap;
  wptr<render::shader> _shader {res::def_skybox_sh.get()};
  wptr<camera3d> _cam {&res::def_cam3d};
};

} // namespace ntf
