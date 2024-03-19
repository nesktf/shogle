#pragma once

#include "level/game_object.hpp"
#include <vector>
#include <functional>

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
  void draw(void);

public:
  std::vector<std::unique_ptr<GameObject>> objs;
  LevelState state;
};

typedef std::function<Level*(void)> LevelCreator;

} // namespace ntf::shogle
