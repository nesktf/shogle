#include "shogle_all.hpp"
#include "input.hpp"
using namespace ntf::shogle;

class TestLevel;

struct DanmakuObject : public SpriteObj {
  DanmakuObject(TestLevel* _level, cref<res::Texture> _tex, cref<res::Shader> _shader) :
    SpriteObj(_tex, _shader),
    level(_level) {}
  TestLevel* level;
  bool should_delete {false};
  glm::vec2 pos {0.0f};
  glm::vec2 vel {0.0f};
  glm::vec2 scale {0.0f};
  float rot {0.0f};
};


class TestLevel : public Level {
private:
  res::Pool<res::Texture, res::Shader> pool;

  std::vector<DanmakuObject> danmaku;

  std::unique_ptr<DanmakuObject> player;
  std::unique_ptr<DanmakuObject> boss;

public: 
  TestLevel() {
    pool.direct_load<res::Texture>({
      {
        .id="cino0",
        .path="res/textures/cirno.png"
      },
      {
        .id="cino1",
        .path="res/textures/cirno_2.jpg"
      }
    });
    pool.direct_load<res::Shader>({
      {
        .id="generic_2d",
        .path="res/shaders/generic_2d"
      }
    });
    player = std::make_unique<DanmakuObject>(this,
      pool.get<res::Texture>("cino0"),
      pool.get<res::Shader>("generic_2d")
    );
    player->pos = {400.0f, 500.0f};
    player->scale = glm::vec2{10.0f};
    player->rot = 0.0f;

    boss = std::make_unique<DanmakuObject>(this,
      pool.get<res::Texture>("cino1"),
      pool.get<res::Shader>("generic_2d")
    );
    boss->set_transform(TransformData{
      .pos = glm::vec3{400.0f, 100.0f, 0.0f},
      .scale = math::sprite_scale(glm::vec2{10.0f}),
      .rot = math::sprite_rot(float{0.0f})
    });
  }
  ~TestLevel() = default;

  void handle_input(float dt) {
    auto& in = InputHandler::instance();
    float speed = 200.0f;
    if (in.is_key_pressed(ntf::shogle::KEY_A)) {
      player->pos.x -= speed*dt;
    } else if (in.is_key_pressed(ntf::shogle::KEY_D)) {
      player->pos.x += speed*dt;
    }
    player->pos.x = std::clamp(player->pos.x, 0.0f, 800.0f);
    if (in.is_key_pressed(ntf::shogle::KEY_W)) {
      player->pos.y -= speed*dt;
    } else if (in.is_key_pressed(ntf::shogle::KEY_S)) {
      player->pos.y += speed*dt;
    }
    player->pos.y = std::clamp(player->pos.y, 0.0f, 600.0f);

    if (in.is_key_pressed(ntf::shogle::KEY_J)) {
      player->rot += 360.0f*dt;
    } else if (in.is_key_pressed(ntf::shogle::KEY_K)){
      player->rot -= 360.0f*dt;
    } else {
      player->rot = 0.0f;
    }
  }

  void update(float dt) override {
    handle_input(dt);
    for (auto& d : danmaku) {
      // d.do_tasks(dt);
      d.draw();
    }

    player->set_transform(TransformData {
      .pos = math::sprite_pos(player->pos),
      .scale = math::sprite_scale(player->scale),
      .rot = math::sprite_rot(player->rot)
    });
    player->draw();

    // boss->do_tasks(dt);
    boss->draw();
  }

public:
  static Level* create(void) { return new TestLevel(); }
};

int main(int argc, char* argv[]) {
  Log::set_level(LogLevel::LOG_VERBOSE);

  auto& shogle = Engine::instance();
  if (shogle.init(Settings{argc, argv, "script/default_settings.lua"})) {
    shogle.start(TestLevel::create);
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

