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
  Level() = default;

  virtual ~Level() = default;

  Level(Level&&) = default; 
  Level& operator=(Level&&) = default;

  Level(const Level&) = delete;
  Level& operator=(const Level&) = delete;

public:
  virtual void update(float dt) = 0;
};

using LevelCreator = std::function<Level*(void)>;

} // namespace ntf::shogle
