#include "scene/model.hpp"

#include "core/engine.hpp"

namespace ntf {

Model::Model(ModelRes* mod, Shader* sha) :
  SceneObj(mod, sha),
  cam(&Shogle::instance().cam3D_default) {}

mat4 Model::model_m_gen(void) {
  mat4 mat{1.0f};

  mat = glm::translate(mat, pos);

  // Euler XYZ
  // TODO: Use quaternions
  mat = glm::rotate(mat, rot.x, vec3{1.0f, 0.0f, 0.0f});
  mat = glm::rotate(mat, rot.y, vec3{0.0f, 1.0f, 0.0f});
  mat = glm::rotate(mat, rot.z, vec3{0.0f, 0.0f, 1.0f});

  mat = glm::scale(mat, scale);

  return mat;
}

void Model::shader_update(Shader* shader, mat4 model_m) {
  shader->use();
  shader->unif_mat4("proj", cam->proj_mat());
  shader->unif_mat4("view", cam->view_mat());
  shader->unif_vec3("view_pos", cam->view_pos());
  shader->unif_mat4("model", model_m);
}

} // namespace ntf
