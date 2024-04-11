#pragma once

#include <vector>
#include <functional>
#include <memory>

namespace ntf {

template<typename TObj>
struct Task {
  virtual ~Task() = default;

  virtual void update(TObj* obj, float dt) = 0;

  bool is_finished {false};
};

template<typename TObj>
struct TaskFun : public Task<TObj> {
  using TaskF = std::function<bool(TObj*, float)>;

  TaskFun(TaskF _fun) :
    fun(_fun) {}

  void update(TObj* obj, float dt) override {
    this->is_finished = fun(obj, dt);
  }

  TaskF fun;
};

template<typename TObj>
class TaskManager {
public:
  using task_t = Task<TObj>;

public:
  void do_tasks(TObj* obj, float dt) {
    // Move new tasks
    for (auto& task : new_tasks) {
      tasks.push_back(std::move(task));
    }
    new_tasks.clear();

    // Do tasks and clear finished tasks
    for (auto& task : tasks) {
      task->update(obj, dt);
    }
    std::erase_if(tasks, [](const auto& task){ return task->is_finished; });
  }

  void add_task(task_t* task) {
    new_tasks.push_back(std::unique_ptr<task_t>{task});
  }

  void add_task(std::unique_ptr<task_t> task) {
    new_tasks.push_back(std::move(task));
  }

  void add_task(TaskFun<TObj>::TaskF task) {
    add_task(std::make_unique<TaskFun<TObj>>(task));
  }

  void clear_tasks(void) {
    for (auto& task : tasks) {
      task->is_finished = true;
    }
  }

private:
  std::vector<std::unique_ptr<task_t>> tasks;
  std::vector<std::unique_ptr<task_t>> new_tasks;
};

} // namespace ntf
