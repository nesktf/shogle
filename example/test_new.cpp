#include <shogle/shogle.hpp>

#include <shogle/render/shaders/generic2d.hpp>
#include <shogle/render/shaders/generic3d.hpp>

#include <shogle/render/meshes/quad.hpp>

#include <shogle/resources/texture.hpp>
#include <shogle/resources/model.hpp>

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

class cirno_model_renderer : public render::drawable3d {
public:
  cirno_model_renderer(scene::transform3d* obj) :
    _obj(obj) {}

public:
  void draw(const scene::camera3d& cam) override {
    for (auto& cirno_mesh : _cirno_fumo) {
      auto& diff = cirno_mesh.materials.begin()->first.tex();
      _shader.set_proj(cam.proj())
        .set_view(mat4{1.0f})
        .set_model(_obj->transf())
        .bind_diffuse(diff)
        .draw(cirno_mesh.mesh);
    }
  }

private:
  scene::transform3d* _obj;
  shaders::generic3d _shader{};
  resources::model _cirno_fumo{"_temp/models/cirno_fumo/cirno_fumo.obj"};
};

class test : public ntf::shogle::application {
public:
  test();

  void draw_event() override;
  void update_event(float dt) override;

private:
  scene::camera3d cam3d {800.0f, 600.0f};
  scene::camera2d cam2d {800.0f, 600.0f};
  scene::transform2d cirno1;
  scene::transform2d cirno2;
  scene::transform2d cirno3;

  scene::transform3d cirno_fumo;

  scene::tasker2d tasks2d;
  scene::tasker3d tasks3d;
  render::renderer2d renderer2d;
  render::renderer3d renderer3d;
};

test::test() : application(800, 600, "test") {
  renderer2d.emplace<cirno_renderer>(std::initializer_list<scene::transform2d*>{&cirno1, &cirno2, &cirno3});
  renderer3d.emplace<cirno_model_renderer>(&cirno_fumo);
  cam2d.set_viewport(win_size()).update();
  cam3d.set_viewport(win_size()).update();

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

  cirno_fumo.set_position(0.0f, -0.25f, -1.0f)
    .set_rotation(-PI*0.25f, vec3{0.0f, 1.0f, 0.0f})
    .set_scale(0.015f)
    .update();

  float t {0.0f};
  tasks2d.add(&cirno1, [center, scale, t](auto& cino, float dt) mutable -> bool {
    t += dt;
    cmplx pos = center + scale*math::expic(PI*t);
    cino.set_rotation(cino.rot() + PI*dt)
      .set_position(pos);
    return false;
  });
  tasks2d.add(&cirno2, [t](auto& cino, float dt) mutable -> bool {
    t += dt;
    cmplx pos = math::expic(-2*PI*t);
    cino.set_rotation(cino.rot() - 3*PI*dt)
      .set_position(pos);
    return false;
  });
  tasks2d.add(&cirno3, [t](auto& cino, float dt) mutable -> bool {
    t += dt;
    cmplx pos = -math::expic(-2*PI*t);
    cino.set_rotation(cino.rot() + PI*dt)
      .set_position(pos);
    return false;
  });
  tasks3d.add(&cirno_fumo, [](auto& cino, float dt) mutable -> bool {
    cino.set_rotation(cino.rot() * math::axisquat(PI*dt, vec3{0.0f,1.0f,0.0f}));
    return false;
  });
}

void test::update_event(float dt) {
  tasks2d.update(dt);
  tasks3d.update(dt);
  cirno1.update();
  cirno_fumo.update();
}

void test::draw_event() {
  gl::clear_viewport(color3{0.2f, 0.2f, 0.2f}, gl::clear::depth);

  gl::set_depth_test(true);
  renderer3d.draw(cam3d);

  gl::set_depth_test(false);
  renderer2d.draw(cam2d);
}

int main() {
  ntf::Log::set_level(ntf::LogLevel::LOG_VERBOSE);
  auto app = test{};
  app.main_loop();

  return EXIT_SUCCESS;
}
