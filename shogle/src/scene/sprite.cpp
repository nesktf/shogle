#include <shogle/scene/sprite.hpp>

#include <shogle/core/log.hpp>

namespace ntf {

sprite::sprite(render::sprite* sprite, render::shader* shader, camera2D& cam) :
  _sprite(sprite), _shader(shader), _cam(cam) {
    assert(sprite && shader);
    _scale = corrected_scale();
}

void sprite::draw(void) { 
  update_shader();
  _sprite->draw(*_shader, _index, inverted_draw); 
}

void sprite::update_shader() {
  _shader->use();
  _shader->set_uniform("proj", _cam.proj());
  _shader->set_uniform("view", _use_screen_space ? mat4{1.0f} : _cam.view());
  _shader->set_uniform("model", this->model_mat());
  _shader->set_uniform("sprite_color", _color);
}

} // namespace ntf
