#pragma once

#include <functional>

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

} // namespace ntf
