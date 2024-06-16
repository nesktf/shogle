#define TASK_INL_HPP
#include <shogle/engine/task.hpp>
#undef TASK_INL_HPP

namespace ntf::shogle {

template<typename T>
void tasker<T>::update(float dt) {
  for (auto& task : _new_tasks) {
    _tasks.push_back(std::move(task));
  }
  _new_tasks.clear();

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
