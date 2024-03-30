#pragma once

#include <glm/mat4x4.hpp>

#include <functional>
#include <queue>

namespace ntf::shogle {

template<typename TObj>
using Task = std::function<bool(TObj*,float,float)>;

template<typename TObj>
class TaskWrapper {
public:
  TaskWrapper(Task<TObj> fun) :
    task(fun) {}

  ~TaskWrapper() = default;

public:
  bool operator()(TObj* obj, float delta_time) {
    elapsed_time += delta_time;
    return task(obj, delta_time, elapsed_time);
  }

public:
  Task<TObj> task;
  float elapsed_time {0.0f};
};

} // namespace ntf::shogle
