#pragma once

#include "singleton.hpp"
#include "settings.hpp"
#include "level/level.hpp"
#include "render/window.hpp"

namespace ntf::shogle {

class Engine : public Singleton<Engine> {
public:
  Engine() = default;
  ~Engine();

public:
  bool init(const Settings& sett);
  void start(LevelCreator creator);

public:
  inline void upd_proj2d_m(float w_width, float w_height) {
    this->proj2d = glm::ortho(0.0f, w_width, w_height, 0.0f, -1.0f, 1.0f);
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
  
public:
  glm::vec3 clear_color, view_pos;
  glm::mat4 proj2d, proj3d, view;

private:
  std::unique_ptr<Window> window;
  std::unique_ptr<Level> level;
  bool should_close;
  float last_frame;
};

} // namespace ntf::shogle
