#pragma once

#include "scene/task.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace ntf {

struct TransformData {
  glm::vec3 pos {0.0f};
  glm::vec3 scale {0.0f};
  glm::vec3 rot {0.0f};
};

template<typename TRenderer>
class SceneObj {
public:
  using ModelGen = glm::mat4(*)(TransformData);
  using task_t = Task<SceneObj<TRenderer>>;

public: 
  template<typename... Args>
  SceneObj(Args&&... args) :
    render_obj(TRenderer{std::forward<Args>(args)...}),
    m_gen(TRenderer::default_modelgen){}

public:
  void update(float dt) {
    // Move new tasks
    for (auto& task : new_tasks) {
      tasks.push_back(std::move(task));
    }
    new_tasks.clear();

    // Do tasks and clear finished tasks
    for (auto& task : tasks) {
      task->update(this, dt);
    }
    std::erase_if(tasks, [](const auto& task){ return task->is_finished; });

    render_obj.draw();
  }

public:
  inline void set_transform(TransformData tr) {
    this->model_m = this->m_gen(tr);
    this->transform = tr;
  }

  inline void add_task(task_t* task) {
    new_tasks.push_back(std::unique_ptr<task_t>{task});
  }
  
  inline void add_task(std::unique_ptr<task_t> task) {
    new_tasks.push_back(std::move(task));
  }

private:
  TRenderer render_obj;

  glm::mat4 model_m {1.0f};
  TransformData transform {};
  ModelGen m_gen;

  std::vector<std::unique_ptr<task_t>> tasks;
  std::vector<std::unique_ptr<task_t>> new_tasks;
};

} // namespace ntf
