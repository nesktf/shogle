#include "core/sprite.hpp"
#include "core/renderer.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace ntf::shogle {

void Sprite::update_model_m(void) {
  glm::mat4 mat{1.0f};

  mat = glm::translate(mat, this->pos_v);
  mat = glm::scale(mat, glm::vec3{this->scale_v, 1.0f});
  mat = glm::rotate(mat, glm::radians(this->rot), glm::vec3{0.0f, 0.0f, 1.0f});

  this->model_m = mat;
}

void Sprite::draw(Shader& shader) {
  Renderer::instance().draw(shader, *this);
}


}
