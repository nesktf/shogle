#pragma once

#include "core/singleton.hpp"
#include "core/settings.hpp"
#include "core/window.hpp"
#include "core/types.hpp"

#include "scene/scene.hpp"
#include "scene/camera2d.hpp"
#include "scene/camera3d.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace ntf {

class Shogle : public Singleton<Shogle> {
public:
  bool init(const Settings& sett);
  void start(SceneCreator creator);

public:
  inline void stop(void) {
    window->close();
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
  uptr<GLWindow> window;

  sceneptr_t level;

  bool should_close {false};
  double last_frame {0.0f};
};

} // namespace ntf::shogle
