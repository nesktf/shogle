#include <shogle/shogle.hpp>

#include <shogle/render/shaders/generic2d.hpp>
#include <shogle/render/meshes/quad.hpp>

#include <shogle/resources/texture.hpp>


using namespace ntf::shogle;

class cirno_renderer : public render::drawable2d {
public:
  cirno_renderer(scene::object2d& cirno) : _cirno(cirno) {
    _cirno_tex.set_filter(gl::texture::filter::linear);
  }

public:
  void draw(const scene::camera2d& cam) override {
    _shader.set_proj(cam.proj())
      .set_view(mat4{1.0f}) // use screen space
      .set_transform(_cirno.transform())
      .set_linear_offset(vec2{1.0f})
      .set_const_offset(vec2{0.0f})
      .set_color(color4{1.0f})
      .bind_texture(_cirno_tex.tex())
      .draw(_quad);
  }

private:
  scene::object2d& _cirno;
  shaders::generic2d _shader{};
  meshes::quad _quad{};
  resources::texture2d _cirno_tex{"_temp/cirno.png"};
};

class test : public ntf::shogle::application {
public:
  test();

  void draw_event() override;
  void update_event(float dt) override;

private:
  scene::camera2d cam {800.0f, 600.0f};
  scene::object2d cirno;

  scene::tasker2d tasks;
  render::renderer2d renderer;
};

test::test() : application(800, 600, "test") {
  renderer.emplace<cirno_renderer>(cirno);

  cmplx center = (cmplx)win_size()*0.5f;
  cirno.set_pos(center)
    .set_rot(0.0f)
    .set_scale(200.0f)
    .update_transform();

  float t {0.0f};
  tasks.add(&cirno, [center, t](auto& cino, float dt) mutable -> bool {
    t += dt;

    cmplx pos = center + 200.0f*math::expic(PI*t);
    cino.set_rot(cino.rot() + PI*dt)
      .set_pos(pos);

    return false;
  });
}

void test::update_event(float dt) {
  tasks.update(dt);
  cirno.update_transform();
}

void test::draw_event() {
  gl::clear_viewport(color3{0.2f, 0.2f, 0.2f});
  renderer.draw(cam);
}

int main() {
  ntf::Log::set_level(ntf::LogLevel::LOG_VERBOSE);
  auto app = test{};
  app.main_loop();

  return EXIT_SUCCESS;
}
