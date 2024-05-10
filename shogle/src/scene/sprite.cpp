#include <shogle/scene/sprite.hpp>

#include <shogle/res/global.hpp>
#include <shogle/core/log.hpp>

namespace ntf {

sprite::sprite(render::sprite* sprite) :
  _sprite(sprite) {
  assert(sprite);
  auto& global {res::global::instance()};
  _shader = global.default_sprite_shader;
  _cam = &global.default_cam2d;

  scale = corrected_scale();
}

sprite::sprite(render::sprite* sprite, render::shader* shader) :
  _sprite(sprite), _shader(shader) {
  assert(sprite && shader);
  auto& global {res::global::instance()};
  _cam = &global.default_cam2d;
  scale = corrected_scale();
}

sprite::sprite(render::sprite* sprite, camera2D* cam) :
  _sprite(sprite), _cam(cam) {
  assert(sprite && cam);
  auto& global {res::global::instance()};
  _shader = global.default_sprite_shader;
  scale = corrected_scale();
}

sprite::sprite(render::sprite* sprite, render::shader* shader, camera2D* cam) :
  _sprite(sprite), _shader(shader), _cam(cam) {
  assert(sprite && shader && cam);
  scale = corrected_scale();
}

void sprite::draw(void) { 
  update_shader();
  render::draw_sprite(*_sprite, *_shader, _index, _inverted_draw);
}

void sprite::update_shader() {
  _shader->use();
  _shader->set_uniform("proj", _cam->proj());
  _shader->set_uniform("view", _use_screen_space ? mat4{1.0f} : _cam->view());
  _shader->set_uniform("model", this->model_mat());
  _shader->set_uniform("sprite_color", color);
}

} // namespace ntf
