#include <shogle/shogle.hpp>

#include <shogle/render/shaders/generic2d.hpp>
#include <shogle/render/meshes/quad.hpp>

#include <shogle/resources/texture.hpp>

using namespace ntf::shogle;

class cirno_renderer : public render::drawable2d {
public:
  cirno_renderer(std::initializer_list<scene::transform2d*> cirnos) :
    _cirnos{cirnos} {
    _cirno_tex.set_filter(gl::texture::filter::linear);
  }

public:
  void draw(const scene::camera2d& cam) override {
    for (auto* cirno : _cirnos) {
      _shader.set_proj(cam.proj())
        .set_view(mat4{1.0f}) // use screen space
        .set_transform(cirno->transf())
        .set_linear_offset(vec2{1.0f})
        .set_const_offset(vec2{0.0f})
        .set_color(color4{1.0f})
        .bind_texture(_cirno_tex.tex())
        .draw(_quad);
    }
  }

private:
  std::vector<scene::transform2d*> _cirnos;
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
  scene::transform2d cirno1;
  scene::transform2d cirno2;
  scene::transform2d cirno3;

  scene::tasker2d tasks;
  render::renderer2d renderer;
};

test::test() : application(800, 600, "test") {
  renderer.emplace<cirno_renderer>(std::initializer_list<scene::transform2d*>{&cirno1, &cirno2, &cirno3});
  cam.set_viewport(win_size()).update();

  float scale {150.0f};
  cmplx center = (cmplx)win_size()*0.5f;

  cirno1.set_position(center)
    .set_rotation(0.0f)
    .set_scale(scale)
    .update();

  cirno2.set_position(0.0f, -1.0f)
    .set_rotation(0.0f)
    .set_scale(0.5f)
    .update();

  cirno3.set_position(0.0f, 0.0f)
    .set_rotation(0.0f)
    .set_scale(0.5f)
    .update();

  cirno1.add_child(&cirno2);
  cirno1.add_child(&cirno3);

  float t {0.0f};
  tasks.add(&cirno1, [center, scale, t](auto& cino, float dt) mutable -> bool {
    t += dt;
    cmplx pos = center + scale*math::expic(PI*t);
    cino.set_rotation(cino.rot() + PI*dt)
      .set_position(pos);
    return false;
  });
  tasks.add(&cirno2, [t](auto& cino, float dt) mutable -> bool {
    t += dt;
    cmplx pos = math::expic(-2*PI*t);
    cino.set_rotation(cino.rot() - 3*PI*dt)
      .set_position(pos);
    return false;
  });
  tasks.add(&cirno3, [t](auto& cino, float dt) mutable -> bool {
    t += dt;
    cmplx pos = -math::expic(-2*PI*t);
    cino.set_rotation(cino.rot() + PI*dt)
      .set_position(pos);
    return false;
  });
}

void test::update_event(float dt) {
  tasks.update(dt);
  cirno1.update();
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
