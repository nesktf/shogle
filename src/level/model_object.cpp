#include "core/model_object.hpp"
#include "core/renderer.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace ntf::shogle {

ModelObject::ModelObject(const Model& _model, Shader& _shader) :
  GameObject(_shader),
  model(std::cref(_model)) {}

void ModelObject::update(float dt) {
  glm::mat4 mat{1.0f};

  mat = glm::translate(mat, this->pos_v);
  mat = glm::scale(mat, this->scale_v);
  mat = glm::rotate(mat, glm::radians(this->rot_v.x), glm::vec3{1.0f, 0.0f, 0.0f});
  mat = glm::rotate(mat, glm::radians(this->rot_v.y), glm::vec3{0.0f, 1.0f, 0.0f});
  mat = glm::rotate(mat, glm::radians(this->rot_v.z), glm::vec3{0.0f, 0.0f, 1.0f});

  this->model_m = mat;
}

void ModelObject::draw(void) const {
  Renderer::instance().draw(this->shader, *this);
}

} // namespace ntf::shogle
