#pragma once

#include "render/drawable.hpp"
#include "task/task.hpp"
#include "log.hpp"

namespace ntf::shogle {

struct TransformData {
  glm::vec3 pos, scale, rot;
};

template<typename TDrawable>
class GameObject {
protected:
  using ModelGenerator = std::function<glm::mat4(TransformData)>;

public: 
  GameObject(TDrawable _obj, ModelGenerator _model_gen) :
    obj(_obj),
    model_gen(_model_gen) {}

  ~GameObject() = default;

public:
  void update(float delta_time) {
    this->elapsed_time += delta_time;

    if (task_queue.empty())
      return;

    auto& task = task_queue.front();
    if (task(this, delta_time)) {
      task_queue.pop();
    }
  }

  inline void update_model(TransformData transf) {
    this->transform = transf;
    this->obj.model_m = model_gen(transf);
  }

  inline void add_task(Task<GameObject> task) {
    this->task_queue.push(std::move(task));
  }

public:
  static inline glm::mat4 model_3d_transform(TransformData tr) {
    glm::mat4 mat{1.0f};

    mat = glm::translate(mat, tr.pos);
    mat = glm::rotate(mat, glm::radians(tr.rot.x), glm::vec3{1.0f, 0.0f, 0.0f});
    mat = glm::rotate(mat, glm::radians(tr.rot.y), glm::vec3{0.0f, 1.0f, 0.0f});
    mat = glm::rotate(mat, glm::radians(tr.rot.z), glm::vec3{0.0f, 0.0f, 1.0f});
    mat = glm::scale(mat, tr.scale);

    return mat;
  }

  static inline glm::mat4 model_2d_transform(TransformData tr) {
    glm::mat4 mat{1.0f};

    mat = glm::translate(mat, glm::vec3{tr.pos.x, tr.pos.y, 0.0f});
    mat = glm::rotate(mat, glm::radians(tr.rot.z), glm::vec3{0.0f, 0.0f, 1.0f});
    mat = glm::scale(mat, glm::vec3{tr.scale.x, tr.scale.y, 1.0f});
    mat = glm::scale(mat, glm::vec3{tr.scale.x, tr.scale.y, 0.0f});

    return mat;
  }

public:
  TDrawable obj;
  TransformData transform;
  ModelGenerator model_gen;

  float elapsed_time {0.0f};
  bool enable {true};

  std::queue<Task<GameObject>> task_queue;
};

} // namespace ntf::shogle
