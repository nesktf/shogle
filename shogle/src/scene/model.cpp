#include <shogle/scene/model.hpp>

#include <shogle/render/shader.hpp>

namespace ntf {

Model::Model(render::model* model, render::shader* shader, Camera3D* cam) :
  _model(model), _shader(shader), _cam(cam) {
    assert(model);
    assert(shader);
    assert(cam);
}

void Model::update(float dt) {
  update_shader();
  Entity3D::update(dt);
  if (draw_on_update) {
    draw();
  }
}

void Model::update_shader() {
  _shader->use();
  _shader->set_uniform("proj", _cam->proj());
  _shader->set_uniform("view", use_screen_space ? mat4{1.0f} : _cam->view());
  _shader->set_uniform("model", this->model());
  _shader->set_uniform("view_pos", _cam->pos());
}


} // namespace ntf
