#include <shogle/scene/sprite.hpp>

#include <shogle/render/shader.hpp>

namespace ntf {

Sprite::Sprite(render::sprite* sprite, render::shader* shader, Camera2D* cam) :
  _sprite(sprite), _shader(shader), _cam(cam) {
    assert(sprite);
    assert(shader);
    assert(cam);
}

void Sprite::update(float dt) {
  update_shader();
  Entity2D::update(dt);
  if (draw_on_update) {
    draw();
  }
}

void Sprite::update_shader() {
  _shader->use();
  _shader->set_uniform("proj", _cam->proj());
  _shader->set_uniform("view", use_screen_space ? mat4{1.0f} : _cam->view());
  _shader->set_uniform("model", this->model());
  _shader->set_uniform("sprite_color", color);
}

} // namespace ntf
