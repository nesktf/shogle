#include <shogle.hpp>

using namespace ntf;

struct test_framebuffer : public scene {

  uptr<dynamic_sprite> rin;
  uptr<model> cirno_fumo;
  uptr<sprite> fbo_sprite;

  bool cirno_anim {false};
  float fbo_time {0.0f};

  render::spritesheet toohus {"_temp/2hus.json"};

  render::model cino {"_temp/models/cirno_fumo/cirno_fumo.obj"};

  render::framebuffer fbo {800, 600};

  void on_create(shogle_state& state) override {
    rin = make_uptr<dynamic_sprite>(toohus["rin_dance"]);
    rin->toggle_screen_space(true);
    set_pos(*rin, {400.0f, 300.0f});
    scale(*rin, 200.0f);

    float t {0.0f};
    rin->add_task([t](auto* obj, float dt) mutable -> bool {
      t += dt;

      if (t > 1/10.0f) {
        t = 0.0f;
        obj->next_index();
      }

      return false;
    });

    cirno_fumo = make_uptr<model>(&cino);
    cirno_fumo->toggle_screen_space(true);
    set_pos(*cirno_fumo, {0.0f, -0.25f, -1.0f});
    set_scale(*cirno_fumo, vec3{0.015f});

    fbo_sprite = make_uptr<sprite>(fbo.get_sprite());
    fbo_sprite->toggle_inverted_draw(true);
    set_pos(*fbo_sprite, {0.0f, 0.0f});
    scale(*fbo_sprite, 200.0f);

    state.input.subscribe(key::ESCAPE, key::PRESS, [&state]() {
      shogle_close_window(state);
    });
    state.input.subscribe(key::SPACE, key::PRESS, [this]() {
      cirno_anim = !cirno_anim;
    });
  }

  void cirno_truco(float t) {
    float ypos = -0.25f + 1.0f*glm::abs(glm::sin(t*PI));
    set_rotation(*cirno_fumo, {t*PI*2.0f, -PI*1.3f + t*PI*3.0f, 0.0f});
    set_pos(*cirno_fumo, vec3{0.0f, ypos, -2.0f});
    set_scale(*cirno_fumo, vec3{0.015f});
  }

  void cirno_vueltitas(float t) {
    set_rotation(*cirno_fumo, -PI*0.5f + t*PI, {0.0f, 1.0f, 0.0f});
    set_pos(*cirno_fumo, vec3{0.0f, -0.25f, -1.0f}+0.25f*vec3{glm::cos(-t*PI), 0.0f, glm::sin(-t*PI)});
    set_scale(*cirno_fumo, vec3{0.015f, 0.0075f+0.0075f*glm::abs(glm::sin(t*PI*4.0f)), 0.015f});
  }

  void rin_vueltitas(float t) {
    set_rotation(*rin, t*PI);
    set_pos(*rin, vec2{400.0f, 300.0f}+200.0f*vec2{glm::cos(-t*PI), glm::sin(-t*PI)});
    set_scale(*rin, 100.0f+200.0f*glm::abs(glm::sin(t*PI))*rin->corrected_scale());
  }

  void update_cameras(shogle_state& state, float dt) {
    auto& in {state.input};

    auto& cam {res::global::instance().default_cam2d};
    auto center {cam.center()};
    if (in.is_key_pressed(key::S)) {
      center.y += 200.0f*dt;
    } else if (in.is_key_pressed(key::W)) {
      center.y -= 200.0f*dt;
    }
    if (in.is_key_pressed(key::D)) {
      center.x += 200.0f*dt;
    } else if (in.is_key_pressed(key::A)) {
      center.x -= 200.0f*dt;
    }

    auto zoom {cam.zoom()};
    if (in.is_key_pressed(key::J)) {
      zoom += vec2{1.0f*dt};
    } else if (in.is_key_pressed(key::K)){
      zoom -= vec2{1.0f*dt};
    }
 
    cam.set_center(center);
    cam.set_zoom(zoom);
    cam.update(dt);
  }

  void update(shogle_state& state, float dt) override {
    update_cameras(state, dt);

    if (cirno_anim) {
      cirno_truco(fbo_time);
    } else {
      cirno_vueltitas(fbo_time);
    }
    cirno_fumo->update(dt);

    fbo_time = periodic_add(fbo_time, dt, 0.0f, 2.0f);
    rin_vueltitas(fbo_time);

    rin->update(dt);
    fbo_sprite->update(dt);
  }

  void draw(shogle_state&) override {
    gl::depth_test(true);
    cirno_fumo->draw();

    gl::depth_test(false);
    {
      auto bind = fbo.bind_scoped();
      gl::clear_viewport(vec4{vec3{glm::abs(glm::sin(PI*fbo_time))}, 1.0f});
      rin->draw();
    }
    fbo_sprite->draw();
  }

  static uptr<scene> create(void) { return make_uptr<test_framebuffer>(); }
};

int main(void) {
  Log::set_level(LogLevel::LOG_VERBOSE);

  auto shogle = shogle_create(800, 600, shogle_gen_title("test_framebuffer"));
  shogle_main_loop(shogle, test_framebuffer::create);

  return EXIT_SUCCESS;
}

