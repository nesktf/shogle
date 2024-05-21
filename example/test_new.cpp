#include "shogle/math/conversions.hpp"
#include "shogle/render/meshes/quad.hpp"
#include "shogle/render/shaders/generic2d.hpp"
#include "shogle/scene/camera.hpp"
#include "shogle/scene/object.hpp"
#include "shogle/scene/task.hpp"
#include <shogle/resources/texture.hpp>

#include <shogle/shogle.hpp>

using namespace ntf::shogle;

class test : public ntf::shogle::application {
public:
  test();
  void render() override;
  void update(float dt) override;

private:
  shaders::generic2d shader;

  meshes::quad quad {meshes::quad::type::normal2d};

  resources::texture2d cino {"_temp/cirno.png"};

  scene::object2d cino_obj;
  scene::camera2d cam;
  cmplx base_pos {400.0f, 300.0f};

  scene::tasker<scene::object2d> tasks;
};

test::test() : application(800, 600, "test") {
  cam.set_viewport(vec2{800, 600}).update_transform();
  cino_obj.set_pos(base_pos)
    .set_rot(0.0f)
    .set_scale(200.0f)
    .update_transform();
  cino.set_filter(gl::texture::filter::linear);

  float t {0.0f};
  tasks.add(&cino_obj, [this, t](auto& cirno, float dt) mutable -> bool {
    t += dt;

    cmplx pos = base_pos + 200.0f*math::expic(PI*t);
    cirno.set_rot(cirno.rot() + PI*dt)
      .set_pos(pos)
      .update_transform();

    return false;
  });
}

void test::update(float dt) {
  tasks.update(dt);
}

void test::render() {
  gl::clear_viewport({0.2f, 0.2f, 0.2f, 1.0f}, false, true);
  shader.set_proj(cam.proj())
    .set_view(mat4{1.0f})
    .set_model(cino_obj.transform())
    .set_linear_offset(vec2{1.0f})
    .set_const_offset(vec2{0.0f})
    .set_color(color4{1.0f})
    .bind_texture(cino.tex())
    .draw(quad);
}

int main() {
  auto app = test{};
  app.main_loop();

  return EXIT_SUCCESS;
}
