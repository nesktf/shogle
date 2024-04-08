#include "scene/model_object.hpp"

namespace ntf {

void ModelObj::update(float dt) {
  tasks.update(dt);

  glm::mat4 mat{1.0f};

  mat = glm::translate(mat, pos);

  // Euler XYZ
  // TODO: Use quaternions
  mat = glm::rotate(mat, rot.x, glm::vec3{1.0f, 0.0f, 0.0f});
  mat = glm::rotate(mat, rot.y, glm::vec3{0.0f, 1.0f, 0.0f});
  mat = glm::rotate(mat, rot.z, glm::vec3{0.0f, 0.0f, 1.0f});

  mat = glm::scale(mat, scale);

  model_m = mat;
}



void ModelObj::draw(void) {
  model.draw(model_m);
}

} // namespace ntf
