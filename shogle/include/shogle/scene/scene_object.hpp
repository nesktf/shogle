#pragma once

#include <shogle/scene/task.hpp>
#include <shogle/res/shader.hpp>

namespace ntf {

class SceneObj {
protected:
  SceneObj() = default;

public:
  virtual ~SceneObj() = default;
  SceneObj(SceneObj&&) = default;
  SceneObj(const SceneObj&) = default;
  SceneObj& operator=(SceneObj&&) = default;
  SceneObj& operator=(const SceneObj&) = default;

public:
  virtual void update(float dt) = 0;

protected:
  virtual mat4 _gen_model(void) = 0;

protected:
  mat4 _model_mat {1.0f};
};

template<typename T, typename TParent>
struct TaskedObj : public TParent, TaskManager<T> {
  template<typename... Args>
  TaskedObj(Args&&... args) :
    TParent(std::forward<Args>(args)...) {}

  void update(float dt) override {
    this->do_tasks(static_cast<T*>(this), dt);
    TParent::update(dt);
  }
};

} // namespace ntf
