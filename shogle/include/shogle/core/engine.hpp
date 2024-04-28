#pragma once

#include <shogle/core/singleton.hpp>
#include <shogle/core/singleton.hpp>
#include <shogle/core/settings.hpp>
#include <shogle/core/types.hpp>

#include <shogle/scene/scene.hpp>

#include <shogle/render/backends/glfw.hpp>
#include <shogle/render/backends/gl.hpp>

namespace ntf {

class Shogle : public Singleton<Shogle> {
public:
  bool init(const Settings& sett);
  void start(SceneCreator creator);

public:
  inline void stop(void) {
    _window->close();
  }

  inline void enable_depth_test(bool flag) {
    render::gl::depth_test(flag);
  }

  inline void enable_blend(bool flag) {
    render::gl::blend(flag);
  }

public:
  vec3 view_pos;
  color3 clear_color;

private:
  uptr<render::window> _window;

  sceneptr_t _scene;

  bool _should_close {false};
  double _last_frame {0.0f};
};

} // namespace ntf::shogle
