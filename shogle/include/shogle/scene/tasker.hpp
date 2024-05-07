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
  using taskid_t = uint;

  struct task_data {
    uptr<task_t> task;
    taskid_t id;
  };

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
    for (auto& curr : _tasks) {
      curr.task->update(static_cast<TName*>(this), dt);
    }
    std::erase_if(_tasks, [](const auto& curr){ return curr.task->is_finished; });

    TParent::update(dt);
  }

public:
  taskid_t add_task(uptr<task_t> task) {
    _new_tasks.push_back(task_data{
      .task = std::move(task), 
      .id = ++_task_counter
    });
    return _task_counter;
  }

  taskid_t add_task(taskfun_t task) {
    return add_task(make_uptr<taskfun_wrapper>(std::move(task)));
  }

  bool end_task(taskid_t id) {
    auto match = [id](auto& task) { return task.id == id; };

    auto it_new = std::find_if(_new_tasks.begin(), _new_tasks.end(), match);
    if (it_new != _new_tasks.end()) {
      _new_tasks.erase(it_new);
      return true;
    }

    auto it = std::find_if(_tasks.begin(), _tasks.end(), match);
    if (it != _tasks.end()) {
      _tasks.erase(it);
      return true;
    }

    return false;
  }

  void clear_tasks(void) {
    _tasks.clear();
  }

private:
  taskid_t _task_counter;
  std::vector<task_data> _tasks;
  std::vector<task_data> _new_tasks;
};

} // namespace ntf
