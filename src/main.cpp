#include "shogle.hpp"
#include "log.hpp"

#include "level/level.hpp"
#include "task/common_sprite.hpp"

using namespace ntf::shogle;

// class ChirunoFumo : public GameObject {
// public:
//   ChirunoFumo(res::Model* model, res::Shader* shader) :
//     GameObject(new Model{model, shader}){
//     this->pos = glm::vec3{0.0f, -0.25f, -1.0f};
//     this->base_scale = 0.015f;
//     this->scale = glm::vec3{base_scale};
//     this->rot = glm::vec3{0.0f};
//     this->jump_speed = 8.0f;
//     this->ang_speed = 200.0f;
//     Log::verbose("[ChirunoFumo] Initialized");
//   }
//   ~ChirunoFumo() {
//     delete this->obj;
//     Log::verbose("[ChirunoFumo] Deleted model");
//   }
//
// public:
//   void update(float dt) override {
//     float half_scale = base_scale / 2.0f;
//     this->scale.y = half_scale + (half_scale*glm::abs(glm::sin(jump_speed*time_elapsed)));
//     this->rot.y += ang_speed * dt;
//
//     model_3d_transform();
//     time_elapsed += dt;
//   }
//
// public:
//   float time_elapsed {0.0f}, base_scale, ang_speed, jump_speed;
// };

class TestLevel : public Level {
public: TestLevel() {
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
      }
    }, [this]{ next_state(); });

    auto* cino = new GameObject<Sprite>(Sprite{
      pool.get_p<res::Texture>("chiruno"),
      pool.get_p<res::Shader>("generic_2d")
    }, GameObject<Sprite>::model_2d_transform);

    cino->add_task(task::spr_init_transform(
      glm::vec2{400.0f, 300.0f},
      glm::vec2{20.0f, 20.0f},
      0.0f
    ));
    cino->add_task(task::spr_rot_right(50.f, 5.0f));
    cino->add_task(task::spr_rot_left(100.0f, 2.5f));

    sprite_obj.emplace(
      std::make_pair("chiruno", std::unique_ptr<GameObject<Sprite>>{cino})
    );
  }
  ~TestLevel() = default;

public:
  void on_load(void) override {
    // auto* cino_fumo = new ChirunoFumo(
    //   pool.get_p<res::Model>("chiruno_fumo"),
    //   pool.get_p<res::Shader>("generic_3d")
    // );
    // objs.emplace(std::make_pair("chiruno_fumo", std::unique_ptr<GameObject>{cino_fumo}));
    //
    // auto* cino = dynamic_cast<ChirunoSprite*>(objs["chiruno"].get());
    // cino->pos = {100.0f, 100.0f};
    // cino->scale = cino->scale/2.0f;
    //
    // auto* cino2 = new ChirunoSprite(
    //   pool.get_p<res::Texture>("chiruno"),
    //   pool.get_p<res::Shader>("generic_2d")
    // );
    // cino2->pos = {700.0f, 100.0f};
    // cino2->scale = cino2->scale/2.0f;
    // objs.emplace(std::make_pair("chiruno2", std::unique_ptr<GameObject>{cino2}));
  }

public:
  static Level* create(void) { return new TestLevel(); }

private:
  res::Pool<res::Texture, res::Shader, res::Model> pool;
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

