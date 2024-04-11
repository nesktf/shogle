#include "scene/model.hpp"

#include "core/engine.hpp"

namespace ntf {

Model::Model(ModelRes* mod, Shader* sha) :
  SceneObj(mod, sha) {}

glm::mat4 Model::model_m_gen(void) {
  glm::mat4 mat{1.0f};

  mat = glm::translate(mat, pos);

  // Euler XYZ
  // TODO: Use quaternions
  mat = glm::rotate(mat, rot.x, glm::vec3{1.0f, 0.0f, 0.0f});
  mat = glm::rotate(mat, rot.y, glm::vec3{0.0f, 1.0f, 0.0f});
  mat = glm::rotate(mat, rot.z, glm::vec3{0.0f, 0.0f, 1.0f});

  mat = glm::scale(mat, scale);

  return mat;
}

void Model::shader_update(Shader* shader, glm::mat4 model_m) {
  const auto& eng = Shogle::instance();

  shader->use();
  shader->unif_mat4("proj", eng.proj3d);
  shader->unif_mat4("view", eng.view);
  shader->unif_vec3("view_pos", eng.view_pos);
  shader->unif_mat4("model", model_m);
}

} // namespace ntf
