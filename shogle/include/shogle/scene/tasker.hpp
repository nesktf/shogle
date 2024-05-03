#pragma once

#include <shogle/core/types.hpp>

#include <vector>
#include <functional>
#include <memory>

namespace ntf {

template<typename T>
concept is_dynamic = requires(T t) {
  { t.update(float{}) };
};

template<typename TParent, typename TName = TParent>
requires(is_dynamic<TParent>)
class tasker : public TParent {
public:
  struct task_t {
    task_t() = default;

    virtual ~task_t() = default;
    task_t(task_t&&) = default;
    task_t(const task_t&) = default;
    task_t& operator=(task_t&&) = default;
    task_t& operator=(const task_t&) = default;

    virtual void update(TName* obj, float dt) = 0;

    bool is_finished {false};
  };

  using taskfun_t = std::function<bool(TName*, float)>;

private:
  struct taskfun_wrapper : public task_t {
    taskfun_wrapper(taskfun_t fun) :
      _fun(fun) {}

    void update(TName* obj, float dt) override {
      this->is_finished = _fun(obj, dt);
    }

    taskfun_t _fun;
  };

public:
  template<typename... Args>
  tasker(Args&&... args) :
    TParent(std::forward<Args>(args)...) {}

public:
  void update(float dt) override {
    // Move new tasks
    for (auto& task : _new_tasks) {
      _tasks.push_back(std::move(task));
    }
    _new_tasks.clear();

    // Do tasks and clear finished tasks
    for (auto& task : _tasks) {
      task->update(static_cast<TName*>(this), dt);
    }
    std::erase_if(_tasks, [](const auto& task){ return task->is_finished; });

    TParent::update(dt);
  }

public:
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