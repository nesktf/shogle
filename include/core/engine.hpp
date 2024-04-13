#pragma once

#include "core/singleton.hpp"
#include "core/settings.hpp"
#include "core/window.hpp"
#include "core/types.hpp"

#include "scene/scene.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace ntf {

class Shogle : public Singleton<Shogle> {
public:
  bool init(const Settings& sett);
  void start(SceneCreator creator);

public:
  inline void upd_proj2d_m(float w_width, float w_height) {
    this->proj2d = glm::ortho(0.0f, w_width, w_height, 0.0f, -10.0f, 1.0f);
  }
  // TODO: Set variable fov??
  inline void upd_proj3d_m(float w_ratio, float fov = 45.0f) {
    this->proj3d = glm::perspective(glm::radians(fov), w_ratio, 0.1f, 100.0f);
  }
  // TODO: Set a proper 3d camera
  inline void upd_view_m() {
    this->view_pos = {0.0f, 0.0f, 0.0f};
    const glm::vec3 view_dir = {0.0f, 0.0f, -1.0f};
    const glm::vec3 view_up =  {0.0f, 1.0f, 0.0f};
    this->view = glm::lookAt(this->view_pos, this->view_pos + view_dir, view_up);
  }

  inline void stop(void) {
    window->close();
  }

  inline void depth_test(bool flag) {
    if (flag) {
      glEnable(GL_DEPTH_TEST);
    } else {
      glDisable(GL_DEPTH_TEST);
    }
  }
  
public:
  vec3 view_pos;
  color3 clear_color;
  mat4 proj2d, proj3d, view;

private:
  uptr<GLWindow> window;

  sceneptr_t level;

  bool should_close {false};
  double last_frame {0.0f};
};

} // namespace ntf::shogle
