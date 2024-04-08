#include "scene/sprite.hpp"

namespace ntf {

void Sprite::update(float dt) {
  tasks.update(this, dt);

  glm::mat4 mat{1.0f};

  mat = glm::translate(mat, glm::vec3{pos, (float)layer});
  mat = glm::rotate(mat, rot, glm::vec3{0.0f, 0.0f, 1.0f});
  mat = glm::scale(mat, glm::vec3{scale, 1.0f});

  model_m = mat;
}

void Sprite::draw(void) {
  sprite.draw(model_m);
}

} // namespace ntf
