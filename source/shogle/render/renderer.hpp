#pragma once

#include <list>
#include <shogle/core/types.hpp>
#include <shogle/scene/object.hpp>

#include <shogle/scene/camera.hpp>

namespace ntf::shogle::render {

template<typename T>
class renderer {
public:
  using cam_t = T;

protected:
  renderer() = default;

public:
  ~renderer() = default;
  renderer(renderer&&) = default;
  renderer(const renderer&) = default;
  renderer& operator=(renderer&&) = default;
  renderer& operator=(const renderer&) = default;

public:
  virtual void draw(cam_t& cam) = 0;
};

using renderer2d = renderer<scene::camera2d>;
using renderer3d = renderer<scene::camera3d>;

} // namespace ntf::shogle::render
