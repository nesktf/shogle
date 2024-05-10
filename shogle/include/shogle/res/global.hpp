#pragma once

#include <shogle/core/singleton.hpp>

#include <shogle/render/shader.hpp>
#include <shogle/scene/camera.hpp>

#include <shogle/res/pool.hpp>


namespace ntf::res {

extern uptr<render::shader> def_sprite_sh;
extern uptr<render::shader> def_model_sh;
extern camera2d def_cam2d;
extern camera3d def_cam3d;

void init_def(void);
void destroy_def(void);

} // namespace ntf::res
