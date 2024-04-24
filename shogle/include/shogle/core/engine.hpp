#pragma once

#include <shogle/core/singleton.hpp>
#include <shogle/core/singleton.hpp>
#include <shogle/core/settings.hpp>
#include <shogle/core/window.hpp>
#include <shogle/core/types.hpp>

#include <shogle/scene/scene.hpp>
#include <shogle/scene/camera2d.hpp>
#include <shogle/scene/camera3d.hpp>

#include <glm/gtc/matrix_transform.hpp>

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
    if (flag) {
      glEnable(GL_DEPTH_TEST);
    } else {
      glDisable(GL_DEPTH_TEST);
    }
  }

  inline void enable_blend(bool flag) {
    if (flag) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    } else {
      glDisable(GL_BLEND);
    }
  }

public:
  vec3 view_pos;
  color3 clear_color;

  Camera2D cam2D_default;
  Camera3D cam3D_default;

private:
  uptr<GLWindow> _window;

  sceneptr_t _scene;

  bool _should_close {false};
  double _last_frame {0.0f};
};

} // namespace ntf::shogle
