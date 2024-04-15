#include "shogle.hpp"

struct TestScene : public ntf::Scene<TestScene> {
  ntf::ResPool<ntf::Shader, ntf::Spritesheet> pool;

  ntf::uptr<ntf::Sprite> rin;
  ntf::uptr<ntf::Sprite> cirno;
  ntf::uptr<ntf::Sprite> sheet;

  TestScene() {
    pool.direct_load<ntf::Shader>({
      .id="generic_2d", .path="res/shaders/generic_2d"
    });
    pool.direct_load<ntf::Spritesheet>({
      .id="2hus", .path="_temp/2hus.json"
    });
    
    sheet = ntf::make_uptr<ntf::Sprite>(
      pool.get<ntf::Spritesheet>("2hus"),
      pool.get<ntf::Shader>("generic_2d")
    );
    sheet->pos = ntf::vec2{0.0f, 0.0f};
    sheet->scale *= 200.0f;

    rin = ntf::make_uptr<ntf::Sprite>(
      pool.get<ntf::Spritesheet>("2hus"),
      "rin_dance",
      pool.get<ntf::Shader>("generic_2d")
    );
    rin->pos = ntf::vec2{-200.0f, 0.0f};
    rin->scale *= 200.0f;

    cirno = ntf::make_uptr<ntf::Sprite>(
      pool.get<ntf::Spritesheet>("2hus"),
      "cirno_fall",
      pool.get<ntf::Shader>("generic_2d")
    );
    cirno->use_screen_space = true;
    cirno->pos = ntf::vec2{100.0f, 100.0f};
    cirno->scale *= 200.0f;

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
    cirno->add_task([t, rate](ntf::Sprite* obj, float dt) mutable {
      t+=dt;
      if(t>rate){
        t = 0.0f;
        obj->next_index();
      }
      return false;
    });

    this->add_task([](auto, float dt) {
      auto& in = ntf::InputHandler::instance();
      auto& cam = ntf::Shogle::instance().cam2D_default;

      auto view = cam.view();

      ntf::vec2 center = view.center;
      float speed = 300.0f;

      if (in.is_key_pressed(ntf::KEY_W)) {
        center.y -= speed*dt;
      } else if (in.is_key_pressed(ntf::KEY_S)) {
        center.y += speed*dt;
      }

      if (in.is_key_pressed(ntf::KEY_A)) {
        center.x -= speed*dt;
      } else if (in.is_key_pressed(ntf::KEY_D)) {
        center.x += speed*dt;
      }

      cam.set_center(center);

      float zoom = view.zoom.x;
      speed = 1.0f;

      if (in.is_key_pressed(ntf::KEY_J)) {
        zoom += speed*dt;
      } else if (in.is_key_pressed(ntf::KEY_K)) {
        zoom -= speed*dt;
      }

      cam.set_zoom(zoom);

      float rot = view.rot;
      speed = 10.0f;

      if (in.is_key_pressed(ntf::KEY_L)) {
        rot += speed*dt;
      }

      cam.set_rot(rot);

      return false;
    });
  };

  void update(float dt) override {
    do_tasks(this, dt);
    sheet->udraw(dt);
    rin->udraw(dt);
    cirno->udraw(dt);
  };
};

int main(int argc, char* argv[]) {
  auto& shogle = ntf::Shogle::instance();
  ntf::Log::set_level(ntf::LogLevel::LOG_VERBOSE);

  if (shogle.init(ntf::Settings{argc, argv, "script/default_settings.lua"})) {
    shogle.enable_depth_test(false);
    shogle.enable_blend(true);
    shogle.start(TestScene::create);
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

