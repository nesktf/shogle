#pragma once

#include "level/game_object.hpp"
#include "resource/pool.hpp"
#include "render/sprite.hpp"
#include "render/model.hpp"

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

  template<typename T>
  using ObjMap = std::unordered_map<res::id_t, std::unique_ptr<T>>;

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

public:
  ObjMap<SpriteObj> sprite_obj;
  ObjMap<ModelObj> model_obj;
  Level::State state;
};
typedef std::function<Level*(void)> LevelCreator;

} // namespace ntf::shogle
