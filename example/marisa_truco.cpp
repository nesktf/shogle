#include <shogle.hpp>

struct car_movement : public ntf::Task<ntf::Model> {
  enum class State {
    Init = 0,
    GoingLeft,
    AtLeft,
    GoingRight,
    AtRight
  };
  car_movement(float _speed, float _limit) : 
    speed(_speed),
    limit(_limit) {}

  void update(ntf::Model* obj, float dt) override {
    switch (state) {
      case State::Init: {
        obj->pos.x = limit;
        this->state = State::GoingLeft;
        break;
      }
      case State::GoingLeft: {
        obj->pos.x -= speed*dt;
        if (obj->pos.x < -1.0f*limit) {
          this->state = State::AtLeft;
        }
        break;
      }
      case State::GoingRight: {
        obj->pos.x += speed*dt;
        if (obj->pos.x > limit) {
          this->state = State::AtRight;
        }
        break;
      }
      case State::AtRight: {
        obj->rot.y = M_PIf*0.5f;
        obj->pos.x = 2.0f;
        this->state = State::GoingLeft;
        break;
      }
      case State::AtLeft: {
        obj->rot.y = -M_PIf*0.5f;
        obj->pos.x = -2.0f;
        this->state = State::GoingRight;
        break;
      }
    }
  }

  State state {State::Init};
  float speed, limit;
};

struct fumo_jump : public ntf::Task<ntf::Model> {
  fumo_jump(float ang_speed, float jump_force, float half_scale) :
    _ang_speed(ang_speed),
    _jump_force(jump_force),
    _half_scale(half_scale) {}

  void update(ntf::Model* obj, float dt) {
    t += dt;
    obj->scale.y = _half_scale + (_half_scale*glm::abs(glm::sin(_jump_force*t)));
    obj->rot.y += _ang_speed*dt;
  }

  float t {0.0f};
  float _ang_speed, _jump_force, _half_scale;
};

struct TestLevel : public ntf::TaskedScene<TestLevel> {
  bool loaded {false};

  ntf::ResPool<ntf::Texture, ntf::Shader, ntf::ModelRes> pool;

  ntf::uptr<ntf::Sprite> cino;

  ntf::uptr<ntf::Model> cino_fumo;
  ntf::uptr<ntf::Model> remu_fumo;
  ntf::uptr<ntf::Model> mari_fumo;
  ntf::uptr<ntf::Model> car;

  TestLevel() {
    pool.direct_load<ntf::Texture>({
      {
        .id="chiruno",
        .path="res/textures/cirno.png"
      }
    });
    pool.direct_load<ntf::Shader>({
      {
        .id="generic_2d",
        .path="res/shaders/generic_2d"
      }, 
      {
        .id="generic_3d",
        .path="res/shaders/generic_3d"
      }
    });
    pool.async_load<ntf::ModelRes>({
      {
        .id="chiruno_fumo",
        .path="_temp/models/cirno_fumo/cirno_fumo.obj"
      }, 
      {
        .id="reimu_fumo",
        .path="_temp/models/reimu_fumo/reimu_fumo.obj"
      },
      {
        .id="marisa_fumo",
        .path="_temp/models/marisa_fumo/marisa_fumo.obj"
      },
      {
        .id="car",
        .path="_temp/models/homer-v/homer-v.obj"
      }
    }, [this]{ on_load(); });

    float t = 0.0f;
    cino = ntf::make_uptr<ntf::Sprite>(
      pool.get<ntf::Texture>("chiruno"),
      pool.get<ntf::Shader>("generic_2d")
    );
    cino->use_screen_space = true;
    cino->pos = ntf::vec2{400.0f, 300.0f};
    cino->scale = ntf::vec2{100.0f};
    cino->rot = 0.0f;
    cino->add_task([](ntf::Sprite* obj, float dt) {
      obj->rot += M_PIf*dt;
      return false;
    });
    cino->add_task([t](ntf::Sprite* obj, float dt) mutable {
      t += dt;
      obj->pos = ntf::vec2{400.0f, 300.0f} + 100.0f*ntf::vec2{glm::cos(10.0f*t), glm::sin(10.0f*t)};
      return false;
    });
  }
  ~TestLevel() = default;

public:
  void update(float dt) override {
    if (!loaded) {
      cino->udraw(dt);
    } else {
      cino_fumo->udraw(dt);
      remu_fumo->udraw(dt);
      mari_fumo->udraw(dt);
      car->udraw(dt);
    }
  }
  
  void on_load(void) {
    cino_fumo = ntf::make_uptr<ntf::Model>(
      pool.get<ntf::ModelRes>("chiruno_fumo"),
      pool.get<ntf::Shader>("generic_3d")
    );
    cino_fumo->use_screen_space = true;
    cino_fumo->pos = ntf::vec3{-0.35f, -0.25f, -1.0f};
    cino_fumo->scale = ntf::vec3{0.015f};
    cino_fumo->rot = ntf::vec3{0.0f};
    cino_fumo->add_task(ntf::make_uptr<fumo_jump>(M_PIf*2.0f, 12.0f, 0.015f*0.5f));

    remu_fumo = ntf::make_uptr<ntf::Model>(
      pool.get<ntf::ModelRes>("reimu_fumo"),
      pool.get<ntf::Shader>("generic_3d")
    );
    remu_fumo->use_screen_space = true;
    remu_fumo->pos = ntf::vec3{0.35f, -0.25f, -1.0f};
    remu_fumo->scale = ntf::vec3{0.015f};
    remu_fumo->rot = ntf::vec3{0.0f};
    remu_fumo->add_task(ntf::make_uptr<fumo_jump>(-M_PIf*2.0f, 12.0f, 0.015f*0.5f));

    mari_fumo = ntf::make_uptr<ntf::Model>(
      pool.get<ntf::ModelRes>("marisa_fumo"),
      pool.get<ntf::Shader>("generic_3d")
    );
    mari_fumo->use_screen_space = true;
    mari_fumo->pos = ntf::vec3{0.0f, -0.25f, -2.0f};
    mari_fumo->scale = ntf::vec3{0.02f};
    mari_fumo->rot = ntf::vec3{0.0f, -M_PIf, 0.0f};
    mari_fumo->add_task([](ntf::Model* obj, float dt) {
      obj->rot.x += M_PIf*2.0f*dt;
      obj->rot.y += M_PIf*3.0f*dt;

      return false;
    });
    float t = 0.0f;
    mari_fumo->add_task([t](ntf::Model* obj, float dt) mutable {
      float base_y = -0.25f;
      float force = 0.75f;
      float speed = 3.15f;
      t += dt;
      obj->pos.y = base_y + (force*glm::abs(glm::sin(speed*t)));
      return false;
    });

    car = ntf::make_uptr<ntf::Model>(
      pool.get<ntf::ModelRes>("car"),
      pool.get<ntf::Shader>("generic_3d")
    );
    car->use_screen_space = true;
    car->pos = ntf::vec3{0.0f, -0.25f, -2.0f};
    car->scale = ntf::vec3{0.3f};
    car->rot = ntf::vec3{0.0f, M_PIf*0.5f, 0.0f};
    car->add_task(ntf::make_uptr<car_movement>(4.4f, 2.2f));

    loaded = true;
    ntf::Shogle::instance().enable_depth_test(true);
  }
};

int main(int argc, char* argv[]) {
  ntf::Log::set_level(ntf::LogLevel::LOG_VERBOSE);
  std::string sett_path {SHOGLE_RESOURCES};
  sett_path += "/script/default_settings.lua";

  auto& shogle = ntf::Shogle::instance();
  if (shogle.init(ntf::Settings{argc, argv, sett_path.c_str()})) {
    shogle.enable_depth_test(false);
    shogle.start(TestLevel::create);
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

