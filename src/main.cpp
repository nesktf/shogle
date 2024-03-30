#include "shogle.hpp"
#include "log.hpp"

#include "level/level.hpp"
#include "task/common_sprite.hpp"
#include "task/common_model.hpp"

using namespace ntf::shogle;

class TestLevel : public Level {
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

    cino->add_task(task::spr_transform(
      glm::vec2{400.0f, 300.0f},
      glm::vec2{20.0f, 20.0f},
      float{0.0f}
    ));
    cino->add_task(task::spr_rot_right(50.f, 5.0f));
    cino->add_task(task::spr_rot_left(100.0f, 2.5f));

    sprite_obj.emplace(res::make_pair_ptr("chiruno", cino));
  }
  ~TestLevel() = default;

public:
  void on_load(void) override {
    sprite_obj["chiruno"]->enable = false;

    auto* cino_fumo = new ModelObj{
      pool.get<res::Model>("chiruno_fumo"),
      pool.get<res::Shader>("generic_3d")
    };
    cino_fumo->add_task(task::mod_transform(
      glm::vec3{-0.25f, -0.25f, -1.0f},
      glm::vec3{0.015f},
      glm::vec3{0.0f}
    ));
    cino_fumo->add_task(task::mod_fumo_jump(200.0f, 10.0f, 2.0f));
    cino_fumo->add_task(task::mod_linear_rel_move(glm::vec3{0.0f, 0.0f, -1.0f}, 1.0f));
    cino_fumo->add_task(task::mod_z_rot(200.0f, 5.0f));
    cino_fumo->add_task(task::mod_linear_abs_move(glm::vec3{0.0f, 0.0f, 0.0f}, 0.5f));
    model_obj.emplace(res::make_pair_ptr("chiruno_fumo", cino_fumo));

    auto* remu_fumo = new ModelObj{
      pool.get<res::Model>("reimu_fumo"),
      pool.get<res::Shader>("generic_3d")
    };
    remu_fumo->add_task(task::mod_transform(
      glm::vec3{0.25f, 0.0f, -1.0f},
      glm::vec3{0.015f},
      glm::vec3{0.0f}
    ));
    remu_fumo->add_task([](auto* obj, float dt, float t) -> bool {
      bool end_task = true;
      static auto fumo = task::mod_fumo_jump(300.0f, 12.0f, 8.5f);

      end_task = end_task && fumo(obj, dt, t);
      auto transform = obj->transform;
      transform.rot.z += 300.0f*dt;
      transform.rot.x += 300.0f*dt;

      obj->update_model(transform);
      
      return end_task;
    });
    model_obj.emplace(res::make_pair_ptr("reimu_fumo", remu_fumo));

    auto* mari_fumo = new ModelObj{
      pool.get<res::Model>("marisa_fumo"),
      pool.get<res::Shader>("generic_3d")
    };
    mari_fumo->add_task(task::mod_transform(
      glm::vec3{-1.0f, -0.25f, -2.0f},
      glm::vec3{0.02f},
      glm::vec3{0.0f, -90.0f, 0.0f}
    ));
    mari_fumo->add_task([](auto* obj, float dt, float t) -> bool {
      bool end_task = true;
      // static auto move = task::mod_linear_rel_move(glm::vec3{2.0f, 0.0f, 0.0f}, 8.5f);
      static auto jump = task::mod_funny_jump(0.5f, 8.5f);
      static float end_x = 1.0f;

      TransformData transform = obj->transform;
      if (transform.pos.x < end_x) {
        transform.pos.x += (2.0f/8.5f)*dt;
        transform.rot.y += 300.0f*dt;
        obj->update_model(transform);
      }

      end_task = end_task && jump(obj, dt, t);

      return end_task;
    });
    mari_fumo->add_task([](auto*,float,float) -> bool {
      auto& shogle = Engine::instance();
      shogle.stop();
      return true;
    });
    model_obj.emplace(res::make_pair_ptr("marisa_fumo", mari_fumo));
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

