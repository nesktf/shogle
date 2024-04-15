#pragma once

#include "core/types.hpp"

#include <vector>
#include <functional>
#include <memory>

namespace ntf {

template<typename T>
struct Task {
  Task() = default;
  virtual ~Task() = default;

  Task(Task&&) = default;
  Task(const Task&) = default;
  Task& operator=(Task&&) = default;
  Task& operator=(const Task&) = default;

  virtual void update(T* obj, float dt) = 0;

  bool is_finished {false};
};

template<typename T>
class TaskManager {
public:
  using task_t = Task<T>;
  using taskfun_t = std::function<bool(T*, float)>;

  struct taskfun_wrapper : public task_t {
    taskfun_wrapper(taskfun_t fun) :
      _fun(fun) {}

    void update(T* obj, float dt) override {
      this->is_finished = _fun(obj, dt);
    }

    taskfun_t _fun;
  };

protected:
  TaskManager() = default;

protected:
  void do_tasks(T* obj, float dt) {
    // Move new tasks
    for (auto& task : _new_tasks) {
      _tasks.push_back(std::move(task));
    }
    _new_tasks.clear();

    // Do tasks and clear finished tasks
    for (auto& task : _tasks) {
      task->update(obj, dt);
    }
    std::erase_if(_tasks, [](const auto& task){ return task->is_finished; });
  }

public:
  void add_task(task_t* task) {
    _new_tasks.push_back(uptr<task_t>{task});
  }

  void add_task(uptr<task_t> task) {
    _new_tasks.push_back(std::move(task));
  }

  void add_task(taskfun_t task) {
    add_task(make_uptr<taskfun_wrapper>(task));
  }

  void end_tasks(void) {
    for (auto& task : _tasks) {
      task->is_finished = true;
    }
  }

private:
  std::vector<uptr<task_t>> _tasks;
  std::vector<uptr<task_t>> _new_tasks;
};

} // namespace ntf
