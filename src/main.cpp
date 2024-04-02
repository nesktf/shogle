#include "shogle_all.hpp"

using namespace ntf::shogle;

struct car_movement : public task::ObjTask<ModelObj> {
  enum class State {
    GoingLeft = 0,
    AtLeft,
    GoingRight,
    AtRight
  };
  car_movement(ModelObj* _obj, float _speed, float _limit) : 
    ObjTask<ModelObj>(_obj),
    speed(_speed),
    limit(_limit) {
    auto transform = _obj->get_transform();
    transform.pos.x = limit;
    _obj->set_transform(transform);
  }
  void task(float dt) override {
    auto transform = obj->get_transform();
    switch (state) {
      case State::GoingLeft: {
        transform.pos.x -= speed*dt;
        if (transform.pos.x < -1.0f*limit) {
          this->state = State::AtLeft;
        }
        break;
      }
      case State::GoingRight: {
        transform.pos.x += speed*dt;
        if (transform.pos.x > limit) {
          this->state = State::AtRight;
        }
        break;
      }
      case State::AtRight: {
        transform.rot.y = 90.0f;
        transform.pos.x = 2.0f;
        this->state = State::GoingLeft;
        break;
      }
      case State::AtLeft: {
        transform.rot.y = -90.0f;
        transform.pos.x = -2.0f;
        this->state = State::GoingRight;
        break;
      }
    }
    obj->set_transform(transform);
  }

  State state {State::GoingLeft};
  float speed, limit;
};

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
      },
      {
        .id="car",
        .path="res/models/homer-v/homer-v.obj"
      }
    }, [this]{ next_state(); });

    auto* cino = new SpriteObj{
      pool.get<res::Texture>("chiruno"),
      pool.get<res::Shader>("generic_2d")
    };
    cino->set_transform(TransformData{
      .pos = math::sprite_pos(glm::vec2{400.0f, 300.0f}),
      .scale = math::sprite_scale(glm::vec2{10.0f}),
      .rot = math::sprite_rot(float{0.0f})
    });
    sprites.emplace(make_pair_ptr("chiruno", cino));
    add_task(task::create<task::spr_rotate>(cino, 300.0f, 10.0f));
    add_task(task::create<task::spr_move_circle>(cino, glm::vec2{400.0f, 300.0f}, 20.0f, 100.0f, 10.0f));
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
      .pos = glm::vec3{-0.35f, -0.25f, -1.0f},
      .scale = glm::vec3{0.015f},
      .rot = glm::vec3{0.0f}
    });
    models.emplace(make_pair_ptr("chiruno_fumo", cino_fumo));
    add_task(task::create<task::mod_fumo_jump>(cino_fumo, 200.0f, 12.0f, anim_time));

    auto* remu_fumo = new ModelObj{
      pool.get<res::Model>("reimu_fumo"),
      pool.get<res::Shader>("generic_3d")
    };
    remu_fumo->set_transform(TransformData{
      .pos = glm::vec3{0.35f, -0.25f, -1.0f},
      .scale = glm::vec3{0.015f},
      .rot = glm::vec3{0.0f}
    });
    models.emplace(make_pair_ptr("reimu_fumo", remu_fumo));
    add_task(task::create<task::mod_fumo_jump>(remu_fumo, -200.0f, 12.0f, anim_time));

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
    add_task(task::create<task::ObjTaskL<ModelObj>>(mari_fumo, [](ModelObj* obj, float dt) -> bool {
      auto transform = obj->get_transform();

      transform.rot.x += 365.0f*dt;
      transform.rot.y += 1.5f*365.0f*dt;
      obj->set_transform(transform);

      return false;
    }));
    add_task(task::create<task::mod_sin_jump>(mari_fumo, 0.75f, 3.2f, anim_time));

    auto* car = new ModelObj {
      pool.get<res::Model>("car"),
      pool.get<res::Shader>("generic_3d")
    };
    car->set_transform(TransformData{
      .pos = glm::vec3{0.0f, -0.25f, -2.0f},
      .scale = glm::vec3{0.3f},
      .rot = glm::vec3{0.0f, 90.0f, 0.0f}
    });
    models.emplace(make_pair_ptr("car", car));
    add_task(task::create<car_movement>(car, 4.4f, 2.2f));
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

