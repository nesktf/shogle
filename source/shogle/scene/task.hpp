#pragma once

#include <shogle/core/types.hpp>
#include <shogle/scene/transform.hpp>

#include <vector>
#include <functional>
#include <memory>

namespace ntf::shogle::scene {

/**
 * @class tas
 * @brief Object task interface
 * @tparam T Object type
 */
template<typename T>
class task {
public:
  /**
   * @brief Build with object pointer
   *
   * @param obj Object pointer
   */
  task(T* obj) :
    _obj(obj) { assert(obj && "Task object can't be null"); }

public:
  virtual ~task() = default;
  task(task&&) = default;
  task(const task&) = default;
  task& operator=(task&&) = default;
  task& operator=(const task&) = default;

public:
  /**
   * @brief Update object. Called each frame.
   *
   * @param dt Delta time (elapsed time)
   */
  virtual void update(float dt) = 0;

public:
  /**
   * @brief Obj reference getter
   *
   * @return Obj reference
   */
  T& obj() { return *_obj; }

  /**
   * @brief Check if task is finished
   *
   * @return Finished flag
   */
  bool finished() const { return _is_finished; }

protected:
  T* _obj;
  bool _is_finished {false};
};

/**
 * @class tasker
 * @brief Task manager
 *
 * @tparam T Object type
 */
template<typename T>
class tasker {
public:
  using task_t = task<T>;
  using taskid = uint;

  /**
   * @class task_data
   * @brief Task data
   *
   */
  struct task_data {
    taskid id;
    uptr<task_t> task;
  };

  using taskfun = std::function<bool(T&, float)>;

  /**
   * @class taskfun_wrapper
   * @brief Task wrapper for lambdas
   *
   */
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
  /**
   * @brief Update all tasks
   *
   * @param dt Delta time (elapsed time)
   */
  void update(float dt);

  /**
   * @brief Add task object
   *
   * @param task Task unique pointer
   * @return Task id
   */
  taskid add(uptr<task_t> task);

  /**
   * @brief Add task lambda
   *
   * @param obj Object pointer associated to lambda
   * @param fun Task lambda
   * @return Task id
   */
  taskid add(T* obj, taskfun fun);

  /**
   * @brief Emplace task object
   *
   * @tparam task Task object type
   * @param args Task object args
   * @return Task id
   */
  template<typename task, typename... Args>
  taskid emplace(Args&&... args);

  /**
   * @brief Force a task to end
   *
   * @param id Task id
   * @return True if task was found and removed. False otherwise.
   */
  bool end(taskid id);

  /**
   * @brief End all tasks
   */
  void clear();

private:
  taskid _task_counter;
  std::vector<task_data> _tasks;
  std::vector<task_data> _new_tasks;
};

/**
  * @brief Task manager for 2d transforms
  */
using tasker2d = tasker<transform2d>;

/**
  * @brief Task manager for 3d transforms
  */
using tasker3d = tasker<transform3d>;

} // namespace ntf::shogle::scene

#ifndef TASK_INL_HPP
#include <shogle/scene/task.inl.hpp>
#endif
