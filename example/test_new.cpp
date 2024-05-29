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
  cirno_model_renderer(scene::transform3d* cirno, scene::transform3d* car) :
    _cirno(cirno), _car(car) {
    for (auto& car_mesh : _car_model) {
      for (auto& mat : car_mesh.materials) {
        mat.first.set_filter(gl::texture::filter::linear);
      }
    }
    _cirno_fumo.begin()->materials.begin()->first.set_filter(gl::texture::filter::linear);
  }

public:
  void draw(const scene::camera3d& cam) override {
    for (auto& car_mesh : _car_model) {
      if (!car_mesh.materials.empty()) {
        _last_mat = &car_mesh.find_material(resources::material_type::diffuse);
      }
      _shader.set_proj(cam.proj())
        .set_view(cam.view())
        .set_model(_car->transf())
        .bind_diffuse(_last_mat->tex())
        .draw(car_mesh.mesh);
    }
    for (auto& cirno_mesh : _cirno_fumo) {
      auto& diff = cirno_mesh.find_material(resources::material_type::diffuse);
      _shader.set_proj(cam.proj())
        .set_view(cam.view())
        .set_model(_cirno->transf())
        .bind_diffuse(diff.tex())
        .draw(cirno_mesh.mesh);
    }
  }

private:
  resources::texture2d* _last_mat;
  scene::transform3d *_cirno, *_car;
  shaders::generic3d _shader{};
  resources::model _cirno_fumo{"_temp/models/cirno_fumo/cirno_fumo.obj"};
  resources::model _car_model{"_temp/models/homer-v/homer-v.obj"};
};

class test : public ntf::shogle::application {
public:
  test();

  void draw_event() override;
  void update_event(float dt) override;
  void input_event(int key, int action) override;
  void poll_input(float dt);
  void viewport_event(size_t w, size_t h) override;
  void cursor_event(double xpos, double ypos) override;
  void scroll_event(double xoff, double yoff) override;

private:
  scene::camera3d cam3d {800.0f, 600.0f};
  scene::camera2d cam2d {800.0f, 600.0f};
  scene::transform2d cirno1;
  scene::transform2d cirno2;
  scene::transform2d cirno3;

  scene::transform3d cirno_fumo;
  scene::transform3d car;

  scene::tasker2d tasks2d;
  scene::tasker3d tasks3d;
  render::renderer2d renderer2d;
  render::renderer3d renderer3d;
  bool enable2d = true;
  quat cirno_init_rot {math::axisquat(math::rad(290.0f), vec3{0.0f, 1.0f, 0.0f})};

  float yaw{-PI*0.5f};
  float pitch{0.0f};
  float last_x{}, last_y{};
  bool first_mouse {true};
};

test::test() : application(800, 600, "test") {
  glfw::set_input_mode(_window, false);
  renderer2d.emplace<cirno_renderer>(std::initializer_list<scene::transform2d*>{&cirno1, &cirno2, &cirno3});
  renderer3d.emplace<cirno_model_renderer>(&cirno_fumo, &car);
  cam3d.set_znear(0.01f).update();

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

  car.set_position(0.0f, 0.0f, -1.0f)
    .set_rotation(quat{1.0f, vec3{0.0f}})
    .set_scale(0.1f)
    .update();

  cirno_fumo.set_position(-0.43f, 0.0f, 0.2f)
    .set_rotation(cirno_init_rot)
    .set_scale(0.028f)
    .update();

  car.add_child(&cirno_fumo);

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
}

void test::update_event(float dt) {
  poll_input(dt);
  tasks3d.update(dt);
  car.update();

  if (!enable2d) return;
  tasks2d.update(dt);
  cirno1.update();
}

void test::draw_event() {
  gl::clear_viewport(color3{0.2f, 0.2f, 0.2f}, gl::clear::depth);

  gl::set_depth_test(true);
  renderer3d.draw(cam3d);

  if (!enable2d) return;
  gl::set_depth_test(false);
  renderer2d.draw(cam2d);
}

void test::poll_input(float dt) {
  float rot_speed = 0.0f;
  if (is_key_pressed(GLFW_KEY_RIGHT)) {
    rot_speed = PI;
  } else if (is_key_pressed(GLFW_KEY_LEFT)) {
    rot_speed = -PI;
  }
  car.set_rotation(car.rot()*math::axisquat(rot_speed*dt, vec3{0.0f,1.0f,0.0f}));

  rot_speed = 0.0f;
  if (is_key_pressed(GLFW_KEY_UP)) {
    rot_speed = -PI;
  } else if (is_key_pressed(GLFW_KEY_DOWN)) {
    rot_speed = PI;
  }
  car.set_rotation(car.rot()*math::axisquat(rot_speed*dt, vec3{1.0f,0.0f,0.0f}));

  float sp = 1.0f*dt;
  vec3 campos = cam3d.pos();
  vec3 camdir = cam3d.dir();
  if (is_key_pressed(GLFW_KEY_W)) {
    campos += sp*camdir;
  } else if (is_key_pressed(GLFW_KEY_S)) {
    campos -= sp*camdir;
  }
  if (is_key_pressed(GLFW_KEY_A)) {
    campos -= glm::normalize(glm::cross(camdir, cam3d.up()))*sp;
  } else if (is_key_pressed(GLFW_KEY_D)) {
    campos += glm::normalize(glm::cross(camdir, cam3d.up()))*sp;
  }
  cam3d.set_position(campos).update();
}

void test::input_event(int key, int action) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    terminate();
  }
  if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
    enable2d = !enable2d;
  }
}

void test::viewport_event(size_t w, size_t h) {
  cam3d.set_viewport(w, h).update();
}

void test::cursor_event(double xpos, double ypos) {
  if (first_mouse) {
    last_x = xpos;
    last_y = ypos;
    first_mouse = false;
  }
  float xoff = xpos - last_x;
  float yoff = last_y - ypos;
  last_x = xpos;
  last_y = ypos;

  float sens = 0.0012f;
  xoff *= sens;
  yoff *= sens;

  yaw += xoff;
  pitch += yoff;
  pitch = std::clamp(pitch, -PI*0.49f, PI*0.49f);

  vec3 dir {
    glm::cos(yaw)*glm::cos(pitch),
    glm::sin(pitch),
    glm::sin(yaw)*glm::cos(pitch)
  };
  cam3d.set_direction(glm::normalize(dir)).update();
}

void test::scroll_event(double, double yoff) {
  float fov = cam3d.fov() - ((float)yoff*0.023f);
  fov = std::clamp(fov, math::rad(1.0f), math::rad(45.0f));
  cam3d.set_fov(fov).update();
}

int main() {
  ntf::Log::set_level(ntf::LogLevel::LOG_VERBOSE);
  auto app = test{};
  app.main_loop();

  return EXIT_SUCCESS;
}
