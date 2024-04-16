#include "scene/model.hpp"

#include "core/engine.hpp"

namespace ntf {

ModelImpl::ModelImpl(const ModelRes* model, const Shader* shader) :
  ModelRenderer(model, shader),
  cam(&Shogle::instance().cam3D_default) {}

void ModelImpl::update(float) {
  _model_mat = _gen_model();
  _shader->use();
  _shader_update();
}

mat4 ModelImpl::_gen_model(void) {
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

void ModelImpl::_shader_update(void) {
  _shader->unif_mat4("proj", cam->proj_mat());
  _shader->unif_mat4("view", use_screen_space ? mat4{1.0f} : cam->view_mat());
  _shader->unif_vec3("view_pos", cam->view_pos());
  _shader->unif_mat4("model", _model_mat);
}

} // namespace ntf
