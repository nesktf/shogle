#pragma once

#include "task/task.hpp"

#include "log.hpp"
#include "types.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace ntf::shogle {

template<typename TDrawable>
class GameObject {
public: 
  GameObject(TDrawable _obj) :
    obj(_obj) {}

  template<typename... Args>
  GameObject(Args&&... args) :
    obj(TDrawable{std::forward<Args>(args)...}) {}

  ~GameObject() = default;

  GameObject(GameObject&&) = default;
  GameObject& operator=(GameObject&&) = default;

  GameObject(const GameObject&) = default;
  GameObject& operator=(const GameObject&) = default;

public:
  void update(float delta_time) {
    elapsed_time += delta_time;

    if (task_queue.empty())
      return;

    auto& task = task_queue.front();
    if (task(this, delta_time)) {
      task_queue.pop();
    }
  }

public:
  inline void update_model(TransformData transf) {
    transform = transf;
    obj.model_m = TDrawable::model_transform(transf);
  }

  inline void add_task(Task<GameObject> task) {
    task_queue.push(TaskWrapper<GameObject>{task});
  }

  inline void draw(void) { obj.draw(); }

public:
  TDrawable obj;
  TransformData transform {};

  float elapsed_time {0.0f};
  bool enable {true};

  std::queue<TaskWrapper<GameObject>> task_queue;
};

} // namespace ntf::shogle
