#pragma once

#include <glm/mat4x4.hpp>

namespace ntf {

class SceneObj {
public:
  virtual ~SceneObj() = default;
  virtual void update(float dt) = 0;
  virtual void draw(void) = 0;

protected:
  glm::mat4 model_m {1.0f};
};

} // namespace ntf
