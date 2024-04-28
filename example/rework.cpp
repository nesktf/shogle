#include <shogle.hpp>

struct TestScene : public ntf::Scene {
  ntf::res::pool<ntf::render::shader, ntf::render::spritesheet, ntf::render::model> pool;

  ntf::uptr<ntf::Sprite> cino;
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

    cino = ntf::make_uptr<ntf::Sprite>(
      pool.get<ntf::render::spritesheet>("2hus")->get("rin_dance"),
      pool.get<ntf::render::shader>("generic_2d")
    );
    cino->use_screen_space = true;
    cino->draw_on_update = true;
    cino->set_pos({400.0f, 300.0f});
    cino->set_scale(200.0f*cino->scale());

    cino_fumo = ntf::make_uptr<ntf::Model>(
      pool.get<ntf::render::model>("cino"),
      pool.get<ntf::render::shader>("generic_3d")
    );
    cino_fumo->use_screen_space = true;
    cino_fumo->draw_on_update = true;
    cino_fumo->set_pos({0.0f, -0.25f, -1.0f});
    cino_fumo->set_scale(ntf::vec3{0.015f});
  }

  void update(float dt) override {
    static float t = 0.0f;
    static size_t i = 0;
    t += dt;
    if (t > 1/30.0f) {
      t = 0.0f;
      cino->set_index(++i);
    }
    auto rot = ntf::quat{M_PIf*0.5f, {0.0f, M_PIf*0.5f, 0.0f}};
    cino_fumo->set_rot(rot);
    ntf::Shogle::instance().enable_depth_test(true);
    cino_fumo->update(dt);
    ntf::Shogle::instance().enable_depth_test(false);
    cino->update(dt);
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
