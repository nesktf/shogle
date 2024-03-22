#pragma once

#include "level/game_object.hpp"
#include "resource/pool.hpp"

#include <functional>
#include <vector>

namespace ntf::shogle {

class Level {
public:
  enum class State {
    Loading = 0,
    Loaded,
    Transition
  };
public:
  Level() { this->state = State::Loading; }
  virtual ~Level() = default;

public:
  void next_state(void);
  void update(float dt);
  void draw(void);

public:
  virtual void on_load(void) {};
  virtual void on_transition(void) {};

  virtual void update_loading(float) {};
  virtual void update_loaded(float) {};

public:
  std::unordered_map<res::id_t, std::unique_ptr<GameObject>> objs;
  Level::State state;
};
typedef std::function<Level*(void)> LevelCreator;

} // namespace ntf::shogle
