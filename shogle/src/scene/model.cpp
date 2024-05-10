#include <shogle/scene/model.hpp>

#include <shogle/res/global.hpp>

namespace ntf {

model::model(render::model* model) :
  _model(model) {
  assert(model);
  auto& global {res::global::instance()};
  _shader = global.default_mesh_shader;
  _cam = &global.default_cam3d;
}

model::model(render::model* model, render::shader* shader) :
  _model(model), _shader(shader) {
  assert(model && shader);
  auto& global {res::global::instance()};
  _cam = &global.default_cam3d;
}

model::model(render::model* model, camera3D* cam) :
  _model(model), _cam(cam) {
  assert(model && cam);
  auto& global {res::global::instance()};
  _shader = global.default_mesh_shader;
}

model::model(render::model* model, render::shader* shader, camera3D* cam) :
  _model(model), _shader(shader), _cam(cam) {
  assert(model && shader && cam);
}

void model::draw(void) {
  update_shader();
  render::draw_model(*_model, *_shader);
}

void model::update_shader() {
  _shader->use();
  _shader->set_uniform("proj", _cam->proj());
  _shader->set_uniform("view", _use_screen_space ? mat4{1.0f} : _cam->view());
  _shader->set_uniform("model", this->model_mat());
  _shader->set_uniform("view_pos", _cam->pos());
}

} // namespace ntf
