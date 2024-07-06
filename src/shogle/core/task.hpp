#pragma once

#include <shogle/core/types.hpp>

#include <vector>
#include <functional>
#include <memory>

namespace ntf::shogle {

template<typename T>
class tasker {
public:
  class task_t {
  public:
    task_t(T* obj) :
      _obj(obj) { assert(obj && "Task object can't be null"); }

  public:
    virtual void update(float dt) = 0;

  public:
    T& obj() { return *_obj; }
    bool finished() const { return _is_finished; }

  public:
    virtual ~task_t() = default;
    task_t(task_t&&) = default;
    task_t(const task_t&) = default;
    task_t& operator=(task_t&&) = default;
    task_t& operator=(const task_t&) = default;

  protected:
    T* _obj;
    bool _is_finished {false};
  };

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
  bool end(taskid id);
  void clear();

  template<typename task, typename... Args>
  taskid emplace(Args&&... args);

private:
  taskid _task_counter;
  std::vector<task_data> _tasks;
  std::vector<task_data> _new_tasks;
};

} // namespace ntf::shogle

#ifndef TASK_INL_HPP
#include <shogle/engine/task.inl.hpp>
#endif
