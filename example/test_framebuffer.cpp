#include <shogle.hpp>

using namespace ntf;

struct test_framebuffer : public scene {
  res::pool<render::shader, render::spritesheet, render::model> pool;

  uptr<dynamic_sprite> rin;
  uptr<model> cirno_fumo;
  uptr<sprite> fbo_sprite;
  render::framebuffer fbo {800, 600};

  bool cirno_anim {false};

  float fbo_t {0.0f};

  test_framebuffer(shogle_state& state) :
    pool(state.loader) {
    pool.direct_request<render::shader>({
      {.id="generic_2d", .path="res/shaders/generic_2d"},
      {.id="generic_3d", .path="res/shaders/generic_3d"}
    });
    pool.direct_request<render::spritesheet>({
      {.id="2hus", .path="_temp/2hus.json"}
    });
    pool.direct_request<render::model>({
      {.id="cino", .path="_temp/models/cirno_fumo/cirno_fumo.obj"}
    });
  }

  void on_create(shogle_state& state) override {
    rin = make_uptr<dynamic_sprite>(
      pool.get<render::spritesheet>("2hus")->get("rin_dance"),
      pool.get<render::shader>("generic_2d"),
      state.cam_2d
    );
    sprite::toggle_screen_space(*rin, true);
    sprite::move(*rin, vec2{400.0f, 300.0f});
    sprite::scale(*rin, rin->corrected_scale(200.0f));

    float t {0.0f};
    rin->add_task([t](auto* obj, float dt) mutable -> bool {
      t += dt;

      if (t > 1/10.0f) {
        t = 0.0f;
        obj->next_index();
      }

      return false;
    });

    cirno_fumo = make_uptr<model>(
      pool.get<render::model>("cino"),
      pool.get<render::shader>("generic_3d"),
      state.cam_3d
    );
    model::toggle_screen_space(*cirno_fumo, true);
    model::move(*cirno_fumo, vec3{0.0f, -0.25f, -1.0f});
    model::scale(*cirno_fumo, vec3{0.015f});

    fbo_sprite = make_uptr<sprite>(
      fbo.get_sprite(),
      pool.get<render::shader>("generic_2d"),
      state.cam_2d
    );
    fbo_sprite->inverted_draw = true;
    sprite::move(*fbo_sprite, vec2{0.0f, 0.0f});
    sprite::scale(*fbo_sprite, fbo_sprite->corrected_scale(200.0f));

    state.input.subscribe(key::ESCAPE, key::PRESS, [&state]() {
      shogle_close_window(state);
    });
    state.input.subscribe(key::SPACE, key::PRESS, [this]() {
      cirno_anim = !cirno_anim;
    });
  }

  void cirno_truco(float t) {
    float ypos = -0.25f + 1.0f*glm::abs(glm::sin(t*PI));
    model::rotate(*cirno_fumo, vec3{t*PI*2.0f, -PI*1.3f + t*PI*3.0f, 0.0f});
    model::move(*cirno_fumo, vec3{0.0f, ypos, -2.0f});
    model::scale(*cirno_fumo, vec3{0.015f});
  }

  void cirno_vueltitas(float t) {
    model::rotate(*cirno_fumo, -PI*0.5f + t*PI, {0.0f, 1.0f, 0.0f});
    model::move(*cirno_fumo, vec3{0.0f, -0.25f, -1.0f}+0.25f*vec3{glm::cos(-t*PI), 0.0f, glm::sin(-t*PI)});
    model::scale(*cirno_fumo, vec3{0.015f, 0.0075f+0.0075f*glm::abs(glm::sin(t*PI*4.0f)), 0.015f});
  }

  void rin_vueltitas(float t) {
    sprite::rotate(*rin, t*PI);
    sprite::move(*rin, vec2{400.0f, 300.0f}+200.0f*vec2{glm::cos(-t*PI), glm::sin(-t*PI)});
    sprite::scale(*rin, 100.0f+200.0f*glm::abs(glm::sin(t*PI))*rin->corrected_scale());
  }

  void update_cameras(shogle_state& state, float dt) {
    auto& in {state.input};

    auto center {state.cam_2d.center()};
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

    auto zoom {state.cam_2d.zoom()};
    if (in.is_key_pressed(key::J)) {
      zoom += vec2{1.0f*dt};
    } else if (in.is_key_pressed(key::K)){
      zoom -= vec2{1.0f*dt};
    }
 
    state.cam_2d.set_center(center);
    state.cam_2d.set_zoom(zoom);

    state.cam_2d.update(dt);
  }

  void update(shogle_state& state, float dt) override {
    update_cameras(state, dt);

    if (cirno_anim) {
      cirno_truco(fbo_t);
    } else {
      cirno_vueltitas(fbo_t);
    }
    cirno_fumo->update(dt);

    fbo_t = periodic_add(fbo_t, dt, 0.0f, 2.0f);
    rin_vueltitas(fbo_t);

    rin->update(dt);
    fbo_sprite->update(dt);
  }

  void draw(shogle_state&) override {
    gl::depth_test(true);
    cirno_fumo->draw();

    gl::depth_test(false);
    {
      auto bind = fbo.bind_scoped();
      gl::clear_viewport(vec4{vec3{glm::abs(glm::sin(PI*fbo_t))}, 1.0f});
      rin->draw();
    }
    fbo_sprite->draw();
  }

  static uptr<scene> create(shogle_state& state) { return make_uptr<test_framebuffer>(state); }
};

int main(int argc, char* argv[]) {
  Log::set_level(LogLevel::LOG_VERBOSE);

  std::string sett_path {SHOGLE_RESOURCES};
  sett_path += "/script/default_settings.lua";
  settings sett{argc, argv, sett_path.c_str()};

  auto shogle = shogle_create(sett.w_width, sett.w_height, sett.w_title);
  gl::blend(true);
  shogle_start(shogle, test_framebuffer::create);

  return EXIT_SUCCESS;
}
