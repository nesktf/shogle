#pragma once

#include "singleton.hpp"
#include "settings.hpp"
#include "level/level.hpp"

namespace ntf::shogle {

class Engine : public Singleton<Engine> {
public:
  Engine() = default;
  ~Engine();

public:
  bool init(const Settings& sett);
  void start(LevelCreator creator);

private:
  std::unique_ptr<Level> level;
  glm::vec3 clear_color;
  bool should_close;
  float last_frame;
};

} // namespace ntf::shogle
