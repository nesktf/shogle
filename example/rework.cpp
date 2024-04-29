#include <shogle.hpp>

#include <shogle/render/framebuffer.hpp>

#define PI M_PIf

struct TestScene : public ntf::Scene {
  ntf::res::pool<ntf::render::shader, ntf::render::spritesheet, ntf::render::model> pool;

  ntf::uptr<ntf::SpriteDynamic> rin;
  ntf::uptr<ntf::Model> cirno_fumo;
  ntf::uptr<ntf::Sprite> fbo_sprite;
  ntf::render::framebuffer fbo;

  bool cirno_anim {false};

  TestScene() :
  fbo(800, 600) {
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

    rin = ntf::make_uptr<ntf::SpriteDynamic>(
      pool.get<ntf::render::spritesheet>("2hus")->get("rin_dance"),
      pool.get<ntf::render::shader>("generic_2d")
    );
    rin->use_screen_space = true;
    rin->draw_on_update = true;
    ntf::move_entity(*rin, ntf::vec2{400.0f, 300.0f});
    ntf::scale_entity(*rin, rin->corrected_scale(200.0f));
    float t {0.0f};
    size_t i {0};
    rin->add_task([t, i](ntf::SpriteDynamic* obj, float dt) mutable -> bool {
      t += dt;

      if (t > 1/10.0f) {
        t = 0.0f;
        obj->set_index(++i);
      }

      return false;
    });

    cirno_fumo = ntf::make_uptr<ntf::Model>(
      pool.get<ntf::render::model>("cino"),
      pool.get<ntf::render::shader>("generic_3d")
    );
    cirno_fumo->use_screen_space = true;
    cirno_fumo->draw_on_update = true;
    ntf::move_entity(*cirno_fumo, ntf::vec3{0.0f, -0.25f, -1.0f});
    ntf::scale_entity(*cirno_fumo, ntf::vec3{0.015f});

    fbo_sprite = ntf::make_uptr<ntf::Sprite>(
      fbo.get_sprite(),
      pool.get<ntf::render::shader>("generic_2d")
    );
    fbo_sprite->draw_on_update = true;
    fbo_sprite->inverted_draw = true;
    ntf::move_entity(*fbo_sprite, ntf::vec2{0.0f, 0.0f});
    ntf::scale_entity(*fbo_sprite, fbo_sprite->corrected_scale(200.0f));

    ntf::input::InputHandler::instance().register_listener(ntf::render::KEY_SPACE, ntf::render::KEY_PRESS, [this]() {
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

  void update(float dt) override {
    static float t = 0.0f;

    auto& in {ntf::input::InputHandler::instance()};
    auto center = ntf::Camera2D::default_cam.center();
    if (in.is_key_pressed(ntf::render::KEY_S)) {
      center.y += 200.0f*dt;
    } else if (in.is_key_pressed(ntf::render::KEY_W)) {
      center.y -= 200.0f*dt;
    }
    if (in.is_key_pressed(ntf::render::KEY_D)) {
      center.x += 200.0f*dt;
    } else if (in.is_key_pressed(ntf::render::KEY_A)) {
      center.x -= 200.0f*dt;
    }

    auto zoom = ntf::Camera2D::default_cam.zoom();
    if (in.is_key_pressed(ntf::render::KEY_J)) {
      zoom += ntf::vec2{1.0f*dt};
    } else if (in.is_key_pressed(ntf::render::KEY_K)){
      zoom -= ntf::vec2{1.0f*dt};
    }
 
    ntf::Camera2D::default_cam.set_center(center);
    ntf::Camera2D::default_cam.set_zoom(zoom);
    ntf::Camera2D::default_cam.update(dt);

    t = ntf::periodic_add(t, dt, 0.0f, 2.0f);

    if (cirno_anim) {
      cirno_truco(t);
    } else {
      cirno_vueltitas(t);
    }
    rin_vueltitas(t);

    ntf::Shogle::instance().enable_depth_test(true);
    cirno_fumo->update(dt);

    ntf::Shogle::instance().enable_depth_test(false);
    {
      auto bind = fbo.bind_scoped();
      ntf::render::gl::clear_viewport(ntf::vec4{ntf::vec3{glm::abs(glm::sin(PI*t))}, 1.0f});
      rin->update(dt);
    }
    fbo_sprite->update(dt);
  }

  static ntf::sceneptr_t create() { return ntf::make_uptr<TestScene>();}
};

int main(int argc, char* argv[]) {
  auto& shogle = ntf::Shogle::instance();
  ntf::Log::set_level(ntf::LogLevel::LOG_VERBOSE);
  std::string sett_path {SHOGLE_RESOURCES};
  sett_path += "/script/default_settings.lua";

  if (shogle.init(ntf::Settings{argc, argv, sett_path.c_str()})) {
    shogle.enable_blend(true);
    shogle.start(TestScene::create);
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}
