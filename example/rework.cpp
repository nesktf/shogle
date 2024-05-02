#include <shogle.hpp>

using ntf::gl;

struct test_scene : public ntf::scene {
  ntf::res::pool<ntf::render::shader, ntf::render::spritesheet, ntf::render::model> pool;

  ntf::uptr<ntf::dynamic_sprite> rin;
  ntf::uptr<ntf::model> cirno_fumo;
  ntf::uptr<ntf::sprite> fbo_sprite;
  ntf::render::framebuffer fbo {800, 600};

  bool cirno_anim {false};

  float fbo_t {0.0f};

  test_scene(ntf::shogle_state& state) :
    pool(state.loader) {
    pool.direct_request<ntf::render::spritesheet>({
      {.id = "2hus", .path="_temp/2hus.json"}
    });
    pool.direct_request<ntf::render::model>({
      {.id = "cino", .path="_temp/models/cirno_fumo/cirno_fumo.obj"}
    });
    pool.direct_request<ntf::render::shader>({
      {.id = "generic_2d", .path = "res/shaders/generic_2d"},
      {.id = "generic_3d", .path = "res/shaders/generic_3d"}
    });
  }

  void on_create(ntf::shogle_state& state) override {
    rin = ntf::make_uptr<ntf::dynamic_sprite>(
      pool.get<ntf::render::spritesheet>("2hus")->get("rin_dance"),
      pool.get<ntf::render::shader>("generic_2d"),
      state.cam_2d
    );
    rin->use_screen_space = true;
    ntf::move_entity(*rin, ntf::vec2{400.0f, 300.0f});
    ntf::scale_entity(*rin, rin->corrected_scale(200.0f));

    float t {0.0f};
    size_t i {0};
    rin->add_task([t, i](auto* obj, float dt) mutable -> bool {
      t += dt;

      if (t > 1/10.0f) {
        t = 0.0f;
        obj->set_index(++i);
      }

      return false;
    });

    cirno_fumo = ntf::make_uptr<ntf::model>(
      pool.get<ntf::render::model>("cino"),
      pool.get<ntf::render::shader>("generic_3d"),
      state.cam_3d
    );
    cirno_fumo->use_screen_space = true;
    ntf::move_entity(*cirno_fumo, ntf::vec3{0.0f, -0.25f, -1.0f});
    ntf::scale_entity(*cirno_fumo, ntf::vec3{0.015f});

    fbo_sprite = ntf::make_uptr<ntf::sprite>(
      fbo.get_sprite(),
      pool.get<ntf::render::shader>("generic_2d"),
      state.cam_2d
    );
    fbo_sprite->inverted_draw = true;
    ntf::move_entity(*fbo_sprite, ntf::vec2{0.0f, 0.0f});
    ntf::scale_entity(*fbo_sprite, fbo_sprite->corrected_scale(200.0f));

    state.input.subscribe(ntf::key::ESCAPE, ntf::key::PRESS, [&state]() {
      ntf::shogle_close_window(state);
    });
    state.input.subscribe(ntf::key::SPACE, ntf::key::PRESS, [this]() {
      cirno_anim = !cirno_anim;
    });
  }

  void cirno_truco(float t) {
    float ypos = -0.25f + 1.0f*glm::abs(glm::sin(t*PI));
    ntf::rotate_entity(*cirno_fumo, ntf::vec3{t*PI*2.0f, -PI*1.3f + t*PI*3.0f, 0.0f});
    ntf::move_entity(*cirno_fumo, ntf::vec3{0.0f, ypos, -2.0f});
    ntf::scale_entity(*cirno_fumo, ntf::vec3{0.015f});
  }

  void cirno_vueltitas(float t) {
    ntf::rotate_entity(*cirno_fumo, -PI*0.5f + t*PI, {0.0f, 1.0f, 0.0f});
    ntf::move_entity(*cirno_fumo, ntf::vec3{0.0f, -0.25f, -1.0f}+0.25f*ntf::vec3{glm::cos(-t*PI), 0.0f, glm::sin(-t*PI)});
    ntf::scale_entity(*cirno_fumo, ntf::vec3{0.015f, 0.0075f+0.0075f*glm::abs(glm::sin(t*PI*4.0f)), 0.015f});
  }

  void rin_vueltitas(float t) {
    ntf::rotate_entity(*rin, t*PI);
    ntf::move_entity(*rin, ntf::vec2{400.0f, 300.0f}+200.0f*ntf::vec2{glm::cos(-t*PI), glm::sin(-t*PI)});
    ntf::scale_entity(*rin, 100.0f+200.0f*glm::abs(glm::sin(t*PI))*rin->corrected_scale());
  }

  void update_cameras(ntf::shogle_state& state, float dt) {
    auto& in {state.input};

    auto center {state.cam_2d.center()};
    if (in.is_key_pressed(ntf::key::S)) {
      center.y += 200.0f*dt;
    } else if (in.is_key_pressed(ntf::key::W)) {
      center.y -= 200.0f*dt;
    }
    if (in.is_key_pressed(ntf::key::D)) {
      center.x += 200.0f*dt;
    } else if (in.is_key_pressed(ntf::key::A)) {
      center.x -= 200.0f*dt;
    }

    auto zoom {state.cam_2d.zoom()};
    if (in.is_key_pressed(ntf::key::J)) {
      zoom += ntf::vec2{1.0f*dt};
    } else if (in.is_key_pressed(ntf::key::K)){
      zoom -= ntf::vec2{1.0f*dt};
    }
 
    state.cam_2d.set_center(center);
    state.cam_2d.set_zoom(zoom);

    state.cam_2d.update(dt);
  }

  void update(ntf::shogle_state& state, float dt) override {
    update_cameras(state, dt);

    if (cirno_anim) {
      cirno_truco(fbo_t);
    } else {
      cirno_vueltitas(fbo_t);
    }
    cirno_fumo->update(dt);

    fbo_t = ntf::periodic_add(fbo_t, dt, 0.0f, 2.0f);
    rin_vueltitas(fbo_t);

    rin->update(dt);
    fbo_sprite->update(dt);
  }

  void draw(ntf::shogle_state&) override {
    gl::depth_test(true);
    cirno_fumo->draw();

    gl::depth_test(false);
    {
      auto bind = fbo.bind_scoped();
      gl::clear_viewport(ntf::vec4{ntf::vec3{glm::abs(glm::sin(PI*fbo_t))}, 1.0f});
      rin->draw();
    }
    fbo_sprite->draw();
  }

  static ntf::uptr<ntf::scene> create(ntf::shogle_state& state) { return ntf::make_uptr<test_scene>(state); }
};

int main(int argc, char* argv[]) {
  ntf::Log::set_level(ntf::LogLevel::LOG_VERBOSE);

  std::string sett_path {SHOGLE_RESOURCES};
  sett_path += "/script/default_settings.lua";
  ntf::settings sett{argc, argv, sett_path.c_str()};

  auto shogle = ntf::shogle_create(sett.w_width, sett.w_height, sett.w_title);
  gl::blend(true);
  ntf::shogle_start(shogle, test_scene::create);

  return EXIT_SUCCESS;
}
