#pragma once

#include "traits.hpp"

#include <glm/mat4x4.hpp>

#include <functional>
#include <queue>
#include <list>

namespace ntf::shogle::task {

class Task {
protected:
  Task() = default;

public:
  virtual ~Task() = default;

  Task(Task&&) = default;
  Task& operator=(Task&&) =default;

  Task(const Task&) = default;
  Task& operator=(const Task&) = default;

public:
  bool operator()(float dt) {
    if (is_finished) {
      // Avoid executing task after finishing
      return true;
    }
    task(dt);
    return is_finished;
  };

public:
  virtual void task(float dt) = 0;

protected:
  void set_task_finished(void) { this->is_finished = true; }
  void set_task_finished(bool flag) { this->is_finished = flag; }

private:
  bool is_finished {false};
};

template<typename TaskObj, typename... Args>
inline Task* create(Args&&... args) {
  return new TaskObj{std::forward<Args>(args)...};
}

template<typename TObj>
requires(is_world_object<TObj>)
class ObjTask : public Task {
protected:
  ObjTask(TObj* _obj) :
    obj(_obj) {}

protected:
  TObj* obj;
};

template<typename TObj>
class ObjTaskL : public ObjTask<TObj> {
private:
  using TaskLambda = std::function<bool(TObj*, float)>;

public:
  ObjTaskL(TObj* _obj, TaskLambda _lambda) : 
    ObjTask<TObj>(_obj),
    lambda(_lambda) {};

  ~ObjTaskL() = default;

public:
  void task(float dt) override {
    this->set_task_finished(lambda(this->obj, dt));
  }

private:
  TaskLambda lambda;
};

} // namespace ntf::shogle
