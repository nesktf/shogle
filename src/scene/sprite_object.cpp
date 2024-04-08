#include "scene/sprite_object.hpp"

namespace ntf {

void SpriteObj::update(float dt) {
  tasks.update(dt);

  glm::mat4 mat{1.0f};

  mat = glm::translate(mat, glm::vec3{pos, 0.0f});
  mat = glm::rotate(mat, rot, glm::vec3{0.0f, 0.0f, 1.0f});
  mat = glm::scale(mat, glm::vec3{scale, 1.0f});

  model_m = mat;
}

void SpriteObj::draw(void) {
  sprite.draw(model_m);
}

} // namespace ntf
