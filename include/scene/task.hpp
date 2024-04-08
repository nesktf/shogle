#pragma once

#include <vector>
#include <functional>
#include <memory>

namespace ntf {

template<typename TObj>
class Task {
public:
  virtual ~Task() = default;

protected:
  virtual void update(TObj* obj, float dt) = 0;

protected:
  bool is_finished {false};
};

template<typename TObj>
class TaskFun : public Task<TObj> {
public:
  using TaskF = std::function<bool(TObj*, float)>;

public:
  TaskFun(TaskF _fun) :
    fun(_fun) {}

public:
  void update(TObj* obj, float dt) override {
    this->is_finished = fun(obj, dt);
  }

private:
  TaskF fun;
};

template<typename TObj>
class TaskManager {
public:
  using task_t = Task<TObj>;

public:
  void update(float dt) {
    // Move new tasks
    for (auto& task : new_tasks) {
      tasks.push_back(std::move(task));
    }
    new_tasks.clear();

    // Do tasks and clear finished tasks
    for (auto& task : tasks) {
      task->update(this, dt);
    }
    std::erase_if(tasks, [](const auto& task){ return task->is_finished; });
  }

public:
  inline void add_task(task_t* task) {
    new_tasks.push_back(std::unique_ptr<task_t>{task});
  }
  
  inline void add_task(std::unique_ptr<task_t> task) {
    new_tasks.push_back(std::move(task));
  }

private:
  std::vector<std::unique_ptr<task_t>> tasks;
  std::vector<std::unique_ptr<task_t>> new_tasks;
};

} // namespace ntf
