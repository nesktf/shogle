#pragma once

#include <shogle/core/types.hpp>

#include <vector>
#include <functional>
#include <memory>

namespace ntf {

class tasker {
public:
  struct task_type {
    task_type() = default;

    virtual void tick(float dt) = 0;

    virtual ~task_type() = default;
    task_type(task_type&&) = default;
    task_type(const task_type&) = default;
    task_type& operator=(task_type&&) = default;
    task_type& operator=(const task_type&) = default;

    bool finished{};
  };

  template<typename F>
  class taskfun : public task_type {
  public:
    taskfun(F fun) : _fun(std::move(fun)) {}

  public:
    void tick(float dt) override { this->finished = _fun(dt); }

  private:
    F _fun;
  };

  template<typename F>
  class timed_event : public task_type {
  public:
    timed_event(float target, F event) : 
      _target(target), _event(std::move(event)) {}

  public:
    void tick(float dt) override {
      _t += dt;
      if (_t >= _target) {
        _event();
        this->finished = true;
      }
    }

  private:
    float _target;
    F _event;
    float _t{0.0f};
  };

public:
  tasker() = default;

public:
  void tick(float dt) {
    for (auto& task : _new_tasks) {
      _tasks.emplace_back(std::move(task));
    }
    _new_tasks.clear();

    for (auto& task : _tasks) {
      task->tick(dt);
    }
    std::erase_if(_tasks, [](const auto& task) { return task->finished; });
  }

  void push_back(uptr<task_type> task) {
    _new_tasks.push_back(std::move(task));
  }

  template<typename T, typename... Args>
  void emplace_back(Args&&... args) {
    _new_tasks.emplace_back(make_uptr<T>(std::forward<Args>(args)...));
  }

  template<typename F, typename = std::enable_if_t<std::is_invocable_r_v<bool, F, float>>>
  void emplace_back(F fun) {
    _new_tasks.emplace_back(make_uptr<taskfun<F>>(std::move(fun)));
  }

  template<typename F, typename = std::enable_if_t<std::is_invocable_v<F>>>
  void emplace_back(float target, F event) {
    _new_tasks.emplace_back(make_uptr<timed_event<F>>(target, std::move(event)));
  }

  void clear() { _tasks.clear(); }

private:
  std::vector<uptr<task_type>> _tasks;
  std::vector<uptr<task_type>> _new_tasks;
};

} // namespace ntf
