#pragma once

#include "level/game_object.hpp"
#include <vector>

namespace ntf::shogle {

enum class LevelState {
  Loading,
  Loaded,
  Unloading
};

class Level {
public:
  Level() { this->state = LevelState::Loading; }
  virtual ~Level() = default;

public:
  void update(float dt);

public:
  virtual void on_load(void) {};

public:
  std::vector<GameObject*> objs;
  LevelState state;
};

} // namespace ntf::shogle
