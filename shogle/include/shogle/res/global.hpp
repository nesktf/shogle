#pragma once

#include <shogle/core/singleton.hpp>

#include <shogle/render/shader.hpp>
#include <shogle/scene/camera.hpp>

namespace ntf::res {

struct global : public Singleton<global> {
  global() = default;

  void init(void);
  void destroy(void);

  render::shader* default_sprite_shader;
  render::shader* default_mesh_shader;
  camera2D default_cam2d;
  camera3D default_cam3d;
};

} // namespace ntf::res
