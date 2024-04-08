#pragma once

#include "scene/task.hpp"

#include <glm/mat4x4.hpp>

namespace ntf {

template<typename TObj>
struct SceneObj {
  SceneObj() = default;
  virtual ~SceneObj() = default;

  SceneObj(SceneObj&&) = default;
  SceneObj& operator=(SceneObj&&) = default;

  SceneObj(const SceneObj&) = default;
  SceneObj& operator=(const SceneObj&) = default;

  virtual void update(float dt) = 0;
  virtual void draw(void) = 0;

  void add_task(TaskManager<TObj>::task_t* task) {
    tasks.add_task(std::unique_ptr<typename TaskManager<TObj>::task_t>{task});
  }
  
  void add_task(std::unique_ptr<typename TaskManager<TObj>::task_t> task) {
    tasks.add_task(std::move(task));
  }

  void add_task(TaskFun<TObj>::TaskF fun) {
    tasks.add_task(std::make_unique<TaskFun<TObj>>(fun));
  }

protected:
  glm::mat4 model_m {1.0f};
  TaskManager<TObj> tasks;
};

} // namespace ntf
