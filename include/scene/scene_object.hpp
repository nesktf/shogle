#pragma once

#include "scene/task.hpp"

#include "res/shader.hpp"

#include <glm/mat4x4.hpp>

namespace ntf {

template<typename TObjName, typename TRenderer>
class SceneObj : public TRenderer, public TaskManager<TObjName> {
protected:
  SceneObj(TRenderer::res_t* res, Shader* sha) :
    TRenderer(res, sha) {}

public:
  virtual ~SceneObj() = default;

  SceneObj(SceneObj&&) = default;
  SceneObj& operator=(SceneObj&&) = default;

  SceneObj(const SceneObj&) = default;
  SceneObj& operator=(const SceneObj&) = default;

protected:
  virtual void shader_update(Shader*, glm::mat4) = 0;
  virtual glm::mat4 model_m_gen(void) = 0;

public:
  void update(float dt) {
    this->do_tasks(static_cast<TObjName*>(this), dt);
    this->_model_m = this->model_m_gen();
    this->shader_update(this->_shader, this->_model_m);
  }

private:
  glm::mat4 _model_m {1.0f};
};

} // namespace ntf
