#include <shogle/scene/model.hpp>

namespace ntf {

model::model(wptr<render::model> model) :
  _model(model) {
  assert(model);
}

model::model(wptr<render::model> model, wptr<render::shader> shader) :
  _model(model), _shader(shader) {
  assert(model && shader);
}

model::model(wptr<render::model> model, wptr<camera3d> cam) :
  _model(model), _cam(cam) {
  assert(model && cam);
}

model::model(wptr<render::model> model, wptr<camera3d> cam, wptr<render::shader> shader) :
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
