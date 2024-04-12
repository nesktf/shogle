#include "shogle.hpp"

#include "res/spritesheet.hpp"


struct TestScene : public ntf::Scene {
  ntf::ResPool<ntf::Shader, ntf::Spritesheet> pool;

  std::unique_ptr<ntf::Sprite> rin;
  std::unique_ptr<ntf::Sprite> cirno;
  std::unique_ptr<ntf::Sprite> sheet;

  TestScene() {
    pool.direct_load<ntf::Shader>({
      {
        .id="generic_2d",
        .path="res/shaders/generic_2d"
      }
    });
    pool.direct_load<ntf::Spritesheet>({
      {
        .id="2hus",
        .path="_temp/2hus.json"
      }
    });

    sheet = std::make_unique<ntf::Sprite>(
      static_cast<ntf::Texture*>(pool.get<ntf::Spritesheet>("2hus")),
      pool.get<ntf::Shader>("generic_2d")
    );
    sheet->pos = glm::vec2{400.0f, 300.0f};
    sheet->scale = 200.0f*sheet->corrected_scale();

    rin = std::make_unique<ntf::Sprite>(
      pool.get<ntf::Spritesheet>("2hus"),
      "rin_dance",
      pool.get<ntf::Shader>("generic_2d")
    );
    rin->pos = glm::vec2{200.0f, 300.0f};
    rin->scale = 200.0f*rin->corrected_scale();
    float t = 0.0f;
    float rate = 1.0f/12.0f;
    rin->add_task([t, rate](ntf::Sprite* obj, float dt) mutable {
      t+=dt;
      if(t>rate){
        t = 0.0f;
        obj->next_index();
      }
      return false;
    });

    cirno = std::make_unique<ntf::Sprite>(
      pool.get<ntf::Spritesheet>("2hus"),
      "cirno_fall",
      pool.get<ntf::Shader>("generic_2d")
    );
    cirno->pos = glm::vec2{600.0f, 300.0f};
    cirno->scale = 200.0f*cirno->corrected_scale();
    cirno->set_index(8);
    cirno->add_task([t, rate](ntf::Sprite* obj, float dt) mutable {
      t+=dt;
      if(t>rate){
        t = 0.0f;
        obj->next_index();
      }
      return false;
    });

  };

  void update(float dt) override {
    sheet->update(dt);
    sheet->draw();

    cirno->update(dt);
    cirno->draw();

    rin->update(dt);
    rin->draw();
  };

  static ntf::sceneptr_t create() {
    return ntf::sceneptr_t{ntf::make_ptr<TestScene>()};
  }

};


int main(int argc, char* argv[]) {
  using namespace ntf;

  Log::set_level(LogLevel::LOG_VERBOSE);

  auto& shogle = Shogle::instance();
  if (shogle.init(Settings{argc, argv, "script/default_settings.lua"})) {
    shogle.depth_test(false);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    shogle.start(TestScene::create);
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

