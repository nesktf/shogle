#include <shogle.hpp>

#define PI M_PIf

struct TestScene : public ntf::Scene {
  ntf::res::pool<ntf::render::shader, ntf::render::spritesheet, ntf::render::model> pool;

  ntf::uptr<ntf::Sprite> rin;
  ntf::uptr<ntf::Model> cino_fumo;

  TestScene() {
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

    rin = ntf::make_uptr<ntf::Sprite>(
      pool.get<ntf::render::spritesheet>("2hus")->get("rin_dance"),
      pool.get<ntf::render::shader>("generic_2d")
    );
    rin->use_screen_space = true;
    rin->draw_on_update = true;
    ntf::move_entity(*rin, ntf::vec2{400.0f, 300.0f});
    ntf::scale_entity(*rin, rin->corrected_scale(200.0f));

    cino_fumo = ntf::make_uptr<ntf::Model>(
      pool.get<ntf::render::model>("cino"),
      pool.get<ntf::render::shader>("generic_3d")
    );
    cino_fumo->use_screen_space = true;
    cino_fumo->draw_on_update = true;
    ntf::move_entity(*cino_fumo, ntf::vec3{0.0f, -0.25f, -1.0f});
    ntf::scale_entity(*cino_fumo, ntf::vec3{0.015f});
  }

  void update(float dt) override {
    static float t = 0.0f;
    static float t2 = 0.0f;
    static size_t i = 0;
    t += dt;
    t2 += dt;
    if (t > 1/10.0f) {
      t = 0.0f;
      rin->set_index(++i);
    }

    ntf::rotate_entity(*cino_fumo, -PI*0.5f + t2*PI, {0.0f, 1.0f, 0.0f});
    ntf::move_entity(*cino_fumo, ntf::vec3{0.0f, -0.25f, -1.0f}+0.25f*ntf::vec3{glm::cos(-t2*PI), 0.0f, glm::sin(-t2*PI)});
    ntf::scale_entity(*cino_fumo, ntf::vec3{0.015f, 0.0075f+0.0075f*glm::abs(glm::sin(t2*PI*4.0f)), 0.015f});

    ntf::rotate_entity(*rin, t2*PI);
    ntf::move_entity(*rin, ntf::vec2{400.0f, 300.0f}+200.0f*ntf::vec2{glm::cos(-t2*PI), glm::sin(-t2*PI)});
    ntf::scale_entity(*rin, 100.0f+200.0f*glm::abs(glm::sin(t2*PI))*rin->corrected_scale());

    ntf::Shogle::instance().enable_depth_test(true);
    cino_fumo->update(dt);

    ntf::Shogle::instance().enable_depth_test(false);
    rin->update(dt);
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
