#pragma once

#include <shogle/core/types.hpp>
#include <shogle/scene/transform.hpp>

#include <vector>
#include <functional>
#include <memory>

namespace ntf::shogle::scene {

// Types
template<typename T>
class task {
public:
  task(T* obj) :
    _obj(obj) { assert(obj && "Task object can't be null"); }

public:
  virtual ~task() = default;
  task(task&&) = default;
  task(const task&) = default;
  task& operator=(task&&) = default;
  task& operator=(const task&) = default;

public:
  virtual void update(float dt) = 0;

public:
  T& obj() { return *_obj; }
  bool finished() const { return _is_finished; }

protected:
  T* _obj;
  bool _is_finished {false};
};

template<typename T>
class tasker {
public:
  using task_t = task<T>;
  using taskid = uint;
  struct task_data {
    taskid id;
    uptr<task_t> task;
  };

  using taskfun = std::function<bool(T&, float)>;
  struct taskfun_wrapper : public task_t {
    taskfun_wrapper(T* obj, taskfun fun) :
      task_t(obj),
      _fun(std::move(fun)) {}

    void update(float dt) override {
      this->_is_finished = _fun(this->obj(), dt);
    }

    taskfun _fun;
  };

public:
  tasker() = default;

public:
  void update(float dt);

  taskid add(uptr<task_t> task);
  taskid add(T* obj, taskfun fun);

  template<typename task, typename... Args>
  taskid emplace(Args&&... args);

  bool end(taskid id);

  void clear();

private:
  taskid _task_counter;
  std::vector<task_data> _tasks;
  std::vector<task_data> _new_tasks;
};

using tasker2d = tasker<transform2d>;
using tasker3d = tasker<transform3d>;


// Inline definitions
template<typename T>
void tasker<T>::update(float dt) {
  // Move new tasks
  for (auto& task : _new_tasks) {
    _tasks.push_back(std::move(task));
  }
  _new_tasks.clear();

  // Do tasks and clear finished tasks
  for (auto& curr : _tasks) {
    curr.task->update(dt);
  }
  std::erase_if(_tasks, [](const auto& curr){ return curr.task->finished(); });
}

template<typename T>
auto tasker<T>::add(uptr<task_t> task) -> taskid {
  _new_tasks.emplace_back(task_data{
    .id = ++_task_counter,
    .task = std::move(task)
  });
  return _task_counter;
}

template<typename T>
auto tasker<T>::add(T* obj, taskfun fun) -> taskid {
  _new_tasks.emplace_back(task_data{
    .id = ++_task_counter,
    .task = make_uptr<taskfun_wrapper>(obj, std::move(fun))
  });
  return _task_counter;
}

template<typename T>
template<typename task, typename... Args>
auto tasker<T>::emplace(Args&&... args) -> taskid {
  _new_tasks.emplace_back(task_data{
    .id = ++_task_counter,
    .task = make_uptr<task>(std::forward<Args>(args)...)
  });
  return _task_counter;
}

template<typename T>
bool tasker<T>::end(taskid id) {
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

template<typename T>
void tasker<T>::clear() { _new_tasks.clear(); _tasks.clear(); }

} // namespace ntf::shogle::scene
