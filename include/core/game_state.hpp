#pragma once

#include "util/singleton.hpp"
#include "core/glfw.hpp"
#include "core/scene.hpp"
#include <glm/vec3.hpp>

namespace ntf:: shogle {

class GameState : public Singleton<GameState> {
public:
  GameState(){}
  ~GameState();

  void init(size_t w_width, size_t w_height, const char* w_name, int argc, char* argv[]);
  void exit_loop(void);

  bool main_loop(void);

private:
  glm::vec3 clear_color;
  Scene* scene;
};


}
