#pragma once

#include "task/task.hpp"

#include "log.hpp"

#include <functional>
#include <vector>

namespace ntf::shogle {

using id_t = std::string;

template<typename TObj>
using ObjMap = std::unordered_map<id_t, std::unique_ptr<TObj>>;

template<typename TObj>
inline std::pair<id_t,std::unique_ptr<TObj>> make_pair_ptr(id_t id, TObj* obj) {
  return std::make_pair(id, std::unique_ptr<TObj>{obj});
}

class Level {
public:
  enum class State {
    Loading = 0,
    Loaded,
    Transition
  };

public:
  Level() :
    state(State::Loading) {}

  virtual ~Level() = default;

  Level(Level&&) = default;
  Level& operator=(Level&&) = default;

  Level(const Level&) = delete;
  Level& operator=(const Level&) = delete;

public:
  virtual void draw(void) = 0;

public:
  virtual void on_load(void) {};
  virtual void on_transition(void) {};

public:
  void next_state(void) {
    switch (this->state) {
      case State::Loading:
        Log::debug("[Level] State change (Loading -> Loaded)");
        this->state = State::Loaded;
        this->on_load();
        break;
      case State::Loaded:
        Log::debug("[Level] State change (Loaded -> Transition)");
        this->state = State::Transition;
        this->on_transition();
        break;
      default:
        break;
    }
  }

  void update_tasks(float dt) {
    for (auto task = tasks.begin(); task != tasks.end();) {
      if ((*task)(dt)) {
        task = tasks.erase(task);
      } else {
        ++task;
      }
    }
  }

  template<typename TObj>
  void add_task(TaskWrapper task) {
    tasks.emplace_back(std::move(task));
  }

protected:
  Level::State state;
  std::vector<TaskWrapper> tasks;
};

using LevelCreator = std::function<Level*(void)>;

} // namespace ntf::shogle
