#pragma once

#include "types.hpp"

#include <glm/mat4x4.hpp>

#include <functional>
#include <queue>
#include <list>

namespace ntf::shogle {

template<typename TObj>
class Task {
public:
  Task(TObj* _obj) :
    obj(_obj) {}

  virtual ~Task() = default;

public:
  virtual bool operator()(float dt) = 0;

protected:
  inline TransformData get_transform(void) {
    return obj->transform;
  }

  inline void set_transform(TransformData data) {
    obj->update_model(data);
  }

protected:
  bool is_finished {false};
  TObj* obj;
};

template<typename TObj>
using TaskLambda = std::function<bool(TObj*,float)>;

template<typename TObj>
class TaskL : public Task<TObj> {
public:
  TaskL(TObj* obj, TaskLambda<TObj> lambda) :
    Task<TObj>(obj),
    fun(lambda) {};

public:
  bool operator()(float dt) override {
    if (this->is_finished) {
      return true;
    }

    this->is_finished = fun(this->obj,dt);
    return false;
  }

protected:
  TaskLambda<TObj> fun;
};

using TaskWrapper = std::function<bool(float)>;

} // namespace ntf::shogle
