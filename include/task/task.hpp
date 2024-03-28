#pragma once

#include <glm/mat4x4.hpp>

#include <functional>
#include <queue>

namespace ntf::shogle {

template<typename TObj>
class Task {
public:
  using TaskFun = std::function<bool(TObj*,float,float)>;

public:
  Task(TaskFun fun) :
    task(fun) {}

  bool operator()(TObj* obj, float delta_time) {
    elapsed_time += delta_time;
    return task(obj, delta_time, elapsed_time);
  }

public:
  TaskFun task;
  float elapsed_time {0.0f};
};

} // namespace ntf::shogle
