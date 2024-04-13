#include "shogle.hpp"

struct TestScene : public ntf::Scene<TestScene> {
  ntf::ResPool<ntf::Shader, ntf::Spritesheet> pool;

  ntf::uptr<ntf::Sprite> rin;
  ntf::uptr<ntf::Sprite> cirno;
  ntf::uptr<ntf::Sprite> sheet;

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

    sheet = ntf::make_uptr<ntf::Sprite>(
      static_cast<ntf::Texture*>(pool.get<ntf::Spritesheet>("2hus")),
      pool.get<ntf::Shader>("generic_2d")
    );
    sheet->pos = ntf::vec2{400.0f, 300.0f};
    sheet->scale = 200.0f*sheet->corrected_scale();

    rin = ntf::make_uptr<ntf::Sprite>(
      pool.get<ntf::Spritesheet>("2hus"),
      "rin_dance",
      pool.get<ntf::Shader>("generic_2d")
    );
    rin->pos = ntf::vec2{200.0f, 300.0f};
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

    cirno = ntf::make_uptr<ntf::Sprite>(
      pool.get<ntf::Spritesheet>("2hus"),
      "cirno_fall",
      pool.get<ntf::Shader>("generic_2d")
    );
    cirno->pos = ntf::vec2{600.0f, 300.0f};
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
    sheet->udraw(dt);
    cirno->udraw(dt);
    rin->udraw(dt);
  };
};


int main(int argc, char* argv[]) {
  auto& shogle = ntf::Shogle::instance();
  ntf::Log::set_level(ntf::LogLevel::LOG_VERBOSE);

  if (shogle.init(ntf::Settings{argc, argv, "script/default_settings.lua"})) {
    shogle.depth_test(false);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    shogle.start(TestScene::create);
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

