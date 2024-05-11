#include <shogle/scene/skybox.hpp>

namespace ntf {

skybox::skybox(wptr<render::cubemap> cubemap) :
  _cubemap(cubemap) {
  assert(cubemap);
}

skybox::skybox(wptr<render::cubemap> cubemap, wptr<render::shader> shader) :
  _cubemap(cubemap), _shader(shader) {
  assert(cubemap && shader);
}

skybox::skybox(wptr<render::cubemap> cubemap, wptr<camera3d> cam) :
  _cubemap(cubemap), _cam(cam) {
  assert(cubemap && cam);
}

skybox::skybox(wptr<render::cubemap> cubemap, wptr<camera3d> cam, wptr<render::shader> shader) :
  _cubemap(cubemap), _shader(shader), _cam(cam) {
  assert(cubemap && cam && shader);
}

void skybox::draw(void) {
  update_shader();
  render::draw_cubemap(*_cubemap, *_shader);
}

void skybox::update_shader(void) {
  auto view  = glm::mat4(glm::mat3(_cam->view()));

  _shader->use();
  _shader->set_uniform("proj", _cam->proj());
  _shader->set_uniform("view", view);
}

} // namespace ntf
