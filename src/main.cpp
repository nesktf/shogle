#include "shogle_2d.hpp"
#include "shogle_3d.hpp"

using namespace ntf::shogle;

class TestLevel : public Level {
private:
  res::Pool<res::Texture, res::Shader, res::Model> pool;
  ObjMap<SpriteObj> sprites;
  ObjMap<ModelObj> models;

public: 
  TestLevel() {
    pool.direct_load<res::Texture>({
      {
        .id="chiruno",
        .path="res/textures/cirno.png"
      }
    });
    pool.direct_load<res::Shader>({
      {
        .id="generic_2d",
        .path="res/shaders/generic_2d"
      }, 
      {
        .id="generic_3d",
        .path="res/shaders/generic_3d"
      }
    });
    pool.async_load<res::Model>({
      {
        .id="chiruno_fumo",
        .path="res/models/cirno_fumo/cirno_fumo.obj"
      },
      {
        .id="reimu_fumo",
        .path="res/models/reimu_fumo/reimu_fumo.obj"
      },
      {
        .id="marisa_fumo",
        .path="res/models/marisa_fumo/marisa_fumo.obj"
      }
    }, [this]{ next_state(); });

    auto* cino = new SpriteObj{
      pool.get<res::Texture>("chiruno"),
      pool.get<res::Shader>("generic_2d")
    };
    cino->set_transform(TransformData{
      .pos = task::sprite_pos(glm::vec2{400.0f, 300.0f}),
      .scale = task::sprite_scale(glm::vec2{10.0f}),
      .rot = task::sprite_rot(float{0.0f})
    });
    sprites.emplace(make_pair_ptr("chiruno", cino));
    add_task<SpriteObj>(task::spr_rotate(cino, 300.0f, 10.0f));
    add_task<SpriteObj>(task::spr_move_circle(cino, {400.0f, 300.0f}, 20.0f, 100.0f, 10.0f));
  }
  ~TestLevel() = default;

public:
  void draw(void) override {
    for (auto& sprite : sprites) {
      sprite.second->draw();
    }
    for (auto& model : models) {
      model.second->draw();
    }
  }
  void on_load(void) override {
    sprites["chiruno"]->enable = false;
    float anim_time = -1.0f;

    auto* cino_fumo = new ModelObj{
      pool.get<res::Model>("chiruno_fumo"),
      pool.get<res::Shader>("generic_3d")
    };
    cino_fumo->set_transform(TransformData{
      .pos = glm::vec3{-0.25f, -0.25f, -1.0f},
      .scale = glm::vec3{0.015f},
      .rot = glm::vec3{0.0f}
    });
    models.emplace(make_pair_ptr("chiruno_fumo", cino_fumo));
    add_task<ModelObj>(task::mod_fumo_jump(cino_fumo, 200.0f, 12.0f, anim_time));

    auto* remu_fumo = new ModelObj{
      pool.get<res::Model>("reimu_fumo"),
      pool.get<res::Shader>("generic_3d")
    };
    remu_fumo->set_transform(TransformData{
      .pos = glm::vec3{0.25f, -0.25f, -1.0f},
      .scale = glm::vec3{0.015f},
      .rot = glm::vec3{0.0f}
    });
    models.emplace(make_pair_ptr("reimu_fumo", remu_fumo));
    add_task<ModelObj>(task::mod_fumo_jump(remu_fumo, -200.0f, 12.0f, anim_time));

    auto* mari_fumo = new ModelObj{
      pool.get<res::Model>("marisa_fumo"),
      pool.get<res::Shader>("generic_3d")
    };
    mari_fumo->set_transform(TransformData{
      .pos = glm::vec3{0.0f, -0.25f, -2.0f},
      .scale = glm::vec3{0.02f},
      .rot = glm::vec3{0.0f, -180.0f, 0.0f}
    });
    models.emplace(make_pair_ptr("marisa_fumo", mari_fumo));
    add_task<ModelObj>(TaskL<ModelObj>{mari_fumo, [](ModelObj* obj, float dt) -> bool {
      auto transform = obj->get_transform();

      transform.rot.x += 200.0f*dt;
      obj->set_transform(transform);

      return false;
    }});
    add_task<ModelObj>(task::mod_sin_jump(mari_fumo, 0.5f, 3.0f, anim_time));
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

