#include "shogle.hpp"

struct TestScene : public ntf::Scene {
  ntf::ResPool<ntf::Texture, ntf::Shader, ntf::ModelRes> pool;

  std::vector<ntf::Sprite> new_danmaku;
  std::vector<std::pair<bool,ntf::Sprite>> danmaku;

  std::unique_ptr<ntf::Sprite> player;
  std::unique_ptr<ntf::Sprite> boss;

  TestScene() {
    pool.direct_load<ntf::Texture>({
      {
        .id="marisa",
        .path="_temp/marisa.png"
      },
      {
        .id="chen",
        .path="_temp/chen.png"
      },
      {
        .id="danmaku",
        .path="_temp/danmaku.png"
      }
    });
    pool.direct_load<ntf::Shader>({
      {
        .id="generic_2d",
        .path="res/shaders/generic_2d"
      }
    });

    float scale_f = 1.2f;

    player = std::make_unique<ntf::Sprite>(
      pool.get<ntf::Texture>("marisa"),
      pool.get<ntf::Shader>("generic_2d")
    );
    player->pos = {400.0f, 700.0f};
    player->scale = scale_f*glm::vec2{32.0f, 48.0f};
    player->add_task([](auto* obj, float dt) -> bool {
      auto& in = ntf::InputHandler::instance();
      float speed = 300.0f;

      if (in.is_key_pressed(ntf::KEY_W)) {
        obj->pos.y -= speed*dt;
      } else if (in.is_key_pressed(ntf::KEY_S)) {
        obj->pos.y += speed*dt;
      }

      if (in.is_key_pressed(ntf::KEY_A)) {
        obj->pos.x -= speed*dt;
      } else if (in.is_key_pressed(ntf::KEY_D)) {
        obj->pos.x += speed*dt;
      }

      obj->pos = glm::clamp(obj->pos, {0.0f, 0.0f}, {800.0f, 600.0f});

      return false;
    });
    player->add_task([this](auto* obj, float) -> bool {
      for (auto& d : danmaku) {
        if (ntf::collision2d(d.second.pos, 16.0f, obj->pos, 16.0f)) {
          // ntf::Log::debug("PICHUUUUN");
          break;
        }
      }
      return false;
    });

    boss = std::make_unique<ntf::Sprite>(
      pool.get<ntf::Texture>("chen"),
      pool.get<ntf::Shader>("generic_2d")
    );
    boss->pos = {400.0f, -100.0f};
    boss->scale = scale_f*glm::vec2{48.0f, 64.0f};
    float t0 = 0.0f;
    boss->add_task([t0, this](ntf::Sprite* obj, float dt) mutable -> bool {
      t0 += dt;
      float total = 1.0f;

      glm::vec2 vel = (glm::vec2{400.0f, 300.0f} - obj->pos)/total;

      obj->pos += vel*dt;

      if (t0 >= total) {
        float t1 = 0.0f;
        float phase = 0.0f;
        boss->add_task([](ntf::Sprite* obj, float dt) -> bool {
          obj->rot += 2.0f*M_PI*dt;
          return false;
        });
        boss->add_task([phase, t1, this](ntf::Sprite* obj, float dt) mutable -> bool {
          t1 += dt;
          phase += M_PI*0.5f*dt;
          int cant = 8;
          float phase_mul = 12.0f;
          if (t1 > 0.05f) {
            t1 = 0.0f;
            for (int i = 0; i < cant; ++i) {
              phase += M_PI/phase_mul;
              auto bullet = ntf::Sprite{
                pool.get<ntf::Texture>("danmaku"),
                pool.get<ntf::Shader>("generic_2d")
              };
              bullet.pos = obj->pos;
              bullet.scale = {16.0f, 16.0f};
              glm::vec2 dir = {glm::cos((float)i*(M_PI/((float)cant*0.5f)) + phase), glm::sin((float)i*(M_PI/((float)cant*0.5f)) + phase)};
              float bul_speed = 200.0f;
              bullet.add_task([bul_speed, dir](ntf::Sprite* o, float dt) -> bool {
                o->pos += bul_speed*dir*dt;;
                return false;
              });
              new_danmaku.push_back(std::move(bullet));
            }
          }
          return false;
          });
        return true;
      }
      return false;
    });
    }

  void update(float dt) override {
    for (auto& d : new_danmaku) {
      danmaku.push_back(std::make_pair(false,std::move(d)));
    }
    new_danmaku.clear();

    boss->update(dt);
    boss->draw();

    player->update(dt);
    player->draw();

    std::for_each(danmaku.begin(), danmaku.end(), [dt](auto& obj) {
      obj.second.update(dt);
      if (obj.second.pos.x > 800.0f || obj.second.pos.x < 0.0f || obj.second.pos.y > 600.0f || obj.second.pos.y < 0.0f) {
        obj.first = true;
      }
      obj.second.draw();
    });

    std::erase_if(danmaku, [](auto& pair) { return pair.first; });
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
    shogle.depth_test(false);
    shogle.start(TestScene::create);
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

