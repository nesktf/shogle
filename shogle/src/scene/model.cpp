#include <shogle/scene/model.hpp>

namespace ntf {

model::model(render::model* model, render::shader* shader, camera3D* cam) :
  _model(model), _shader(shader), _cam(cam) {
    assert(model && shader && cam);
}

void model::draw(void) {
  update_shader();
  _model->draw(*_shader);
}

void model::update_shader() {
  _shader->use();
  _shader->set_uniform("proj", _cam->proj());
  _shader->set_uniform("view", _use_screen_space ? mat4{1.0f} : _cam->view());
  _shader->set_uniform("model", this->model_mat());
  _shader->set_uniform("view_pos", _cam->pos());
}

} // namespace ntf
