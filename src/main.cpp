#include "shogle.hpp"

struct TestScene : public ntf::Scene {
  ntf::ResPool<ntf::Texture, ntf::Shader, ntf::ModelRes> pool;
  std::vector<ntf::Sprite> spr;
  std::vector<ntf::Model> mod;

  TestScene() {
    pool.direct_load<ntf::Texture>({
      {
        .id="cirno0",
        .path="res/textures/cirno.png"
      },
      {
        .id="cirno1",
        .path="res/textures/cirno_2.jpg"
      }
    });
    pool.direct_load<ntf::Shader>({
      {
        .id="generic_2d",
        .path="res/shaders/generic_2d"
      },
      {
        .id="generic_3d",
        .path="res/shaders/generic_3d"
      }
    });
    pool.direct_load<ntf::ModelRes>({
      {
        .id="cirno_fumo",
        .path="res/models/cirno_fumo/cirno_fumo.obj"
      }
    });
    spr.emplace_back(ntf::Sprite{
      pool.get<ntf::Texture>("cirno0"),
      pool.get<ntf::Shader>("generic_2d")
    });
    spr.back().pos = {400.0f, 300.0f};
    spr.back().scale = glm::vec2{200.0f};
    spr.back().add_task([](ntf::Sprite* obj, float dt) -> bool {
      auto& in = ntf::InputHandler::instance();
      float speed = 400.0f;

      if (in.is_key_pressed(ntf::KEY_A)) {
        obj->pos.x -= speed*dt;
      } else if (in.is_key_pressed(ntf::KEY_D)) {
        obj->pos.x += speed*dt;
      }
      obj->pos.x = std::clamp(obj->pos.x, 0.0f, 800.0f);

      if (in.is_key_pressed(ntf::KEY_W)) {
        obj->pos.y -= speed*dt;
      } else if (in.is_key_pressed(ntf::KEY_S)) {
        obj->pos.y += speed*dt;
      }
      obj->pos.y = std::clamp(obj->pos.y, 0.0f, 600.0f);

      return false;
    });

    spr.emplace_back(ntf::Sprite{
      pool.get<ntf::Texture>("cirno1"),
      pool.get<ntf::Shader>("generic_2d")
    });
    spr.back().pos = {400.00f, 250.0f};
    spr.back().scale = glm::vec2{200.0f};

    mod.emplace_back(ntf::Model{
      pool.get<ntf::ModelRes>("cirno_fumo"),
      pool.get<ntf::Shader>("generic_3d")
    });
    mod.back().pos = {0.0f, -0.25f, -1.0f};
    mod.back().scale = glm::vec3{0.015f};
    mod.back().rot = glm::vec3{0.0f};

    float half_scale = 0.015f*0.5f;
    float t = 0.0f;
    mod.back().add_task([t, half_scale](ntf::Model* obj, float dt) mutable -> bool {
      t += dt;
      obj->rot.y += 2.0f*M_PI*dt;
      obj->scale.y = half_scale + (half_scale*glm::abs(glm::sin(10.0f*t)));
      return false;
    });
  }

  void update(float dt) override {
    auto& eng = ntf::Shogle::instance();

    eng.depth_test(true);
    std::for_each(mod.begin(), mod.end(), [dt](ntf::Model& obj) {
      obj.update(dt);
      obj.draw();
    });

    eng.depth_test(false);
    std::for_each(spr.begin(), spr.end(), [dt](ntf::Sprite& obj) {
      obj.rot += M_PI * dt;
      obj.update(dt);
      obj.draw();
    });
  }

  static ntf::sceneptr_t create() {
    return ntf::sceneptr_t{ntf::make_ptr<TestScene>()};
  }
};

int main(int argc, char* argv[]) {
  using namespace ntf;

  Log::set_level(LogLevel::LOG_VERBOSE);

  auto& shogle = Shogle::instance();
  if (shogle.init(Settings{argc, argv, "script/default_settings.lua"})) {
    shogle.start(TestScene::create);
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

