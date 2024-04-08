#include "shogle.hpp"

struct TestScene : public ntf::Scene {
  ntf::ResPool<ntf::Texture, ntf::Shader> pool;
  std::vector<ntf::Sprite> objs;

  TestScene() {
    pool.direct_load<ntf::Texture>({
      {
        .id="cirno",
        .path="res/textures/cirno.png"
      }
    });
    pool.direct_load<ntf::Shader>({
      {
        .id="generic_2d",
        .path="res/shaders/generic_2d"
      }
    });

    objs.emplace_back(ntf::Sprite{
      pool.get<ntf::Texture>("cirno"),
      pool.get<ntf::Shader>("generic_2d")
    });
    objs.back().pos = {400.0f, 300.0f};
    objs.back().scale = glm::vec2{200.0f};
  }

  void update(float dt) override {
    std::for_each(objs.begin(), objs.end(), [dt](ntf::Sprite& obj) {
      obj.rot += M_PI * dt;
      obj.update(dt);
      obj.draw();
    });
  }

  static ntf::sceneptr_t create() {
    return ntf::sceneptr_t{new TestScene()};
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

