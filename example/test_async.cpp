#include <shogle.hpp>

using namespace ntf;

struct car_movement : public dynamic_model::task_t {
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

  void update(dynamic_model* obj, float dt) override {
    switch (state) {
      case State::Init: {
        obj->_pos.x = limit;
        this->state = State::GoingLeft;
        break;
      }
      case State::GoingLeft: {
        obj->_pos.x -= speed*dt;
        if (obj->_pos.x < -1.0f*limit) {
          this->state = State::AtLeft;
        }
        break;
      }
      case State::GoingRight: {
        obj->_pos.x += speed*dt;
        if (obj->_pos.x > limit) {
          this->state = State::AtRight;
        }
        break;
      }
      case State::AtRight: {
        vec3 rot = glm::eulerAngles(obj->_rot);
        rot.y = PI*0.5f;
        model::rotate(*obj, rot);

        obj->_pos.x = 2.0f;
        this->state = State::GoingLeft;
        break;
      }
      case State::AtLeft: {
        vec3 rot = glm::eulerAngles(obj->_rot);
        rot.y = -PI*0.5f;
        model::rotate(*obj, rot);

        obj->_pos.x = -2.0f;
        this->state = State::GoingRight;
        break;
      }
    }
  }

  State state {State::Init};
  float speed, limit;
};

struct fumo_jump : public dynamic_model::task_t {
  fumo_jump(float ang_speed, float jump_force, float half_scale) :
    _ang_speed(ang_speed),
    _jump_force(jump_force),
    _half_scale(half_scale) {}

  void update(dynamic_model* obj, float dt) override {
    t += dt;
    obj->_scale.y = _half_scale + (_half_scale*glm::abs(glm::sin(_jump_force*t)));

    model::rotate(*obj, {0.0f, _ang_speed*t, 0.0f});
  }

  float t {0.0f};
  float _ang_speed, _jump_force, _half_scale;
};

struct test_async : public scene {
  res::pool<render::shader, render::spritesheet, render::model> pool;

  uptr<dynamic_sprite> cino;

  uptr<dynamic_model> cino_fumo;
  uptr<dynamic_model> remu_fumo;
  uptr<dynamic_model> mari_fumo;
  uptr<dynamic_model> car;

  bool loaded {false};

  test_async(shogle_state& state) :
    pool(state.loader) {
    pool.direct_request<render::shader>({
      {.id="generic_2d", .path="res/shaders/generic_2d"}, 
      {.id="generic_3d", .path="res/shaders/generic_3d"}
    });
    pool.direct_request<render::spritesheet>({
      {.id="chiruno", .path="_temp/cirno.json"}
    });
    pool.async_request<render::model>({
      {.id="chiruno_fumo", .path="_temp/models/cirno_fumo/cirno_fumo.obj"}, 
      {.id="reimu_fumo", .path="_temp/models/reimu_fumo/reimu_fumo.obj"},
      {.id="marisa_fumo", .path="_temp/models/marisa_fumo/marisa_fumo.obj"},
      {.id="car", .path="_temp/models/homer-v/homer-v.obj" }
    }, [this, &state]{ on_load(state); });
  }

  void on_create(shogle_state& state) override {
    float t = 0.0f;
    cino = make_uptr<dynamic_sprite>(
      pool.get<render::spritesheet>("chiruno")->get("cirno"),
      pool.get<render::shader>("generic_2d"),
      state.cam_2d
    );
    sprite::toggle_screen_space(*cino, true);
    sprite::move(*cino, {400.0f, 300.0f});
    sprite::scale(*cino, vec2{100.0f});

    cino->add_task([t](dynamic_sprite* obj, float dt) mutable {
      t += dt;
      sprite::rotate(*obj, PI*t);
      return false;
    });
    cino->add_task([t](dynamic_sprite* obj, float dt) mutable {
      t += dt;
      sprite::move(*obj, vec2{400.0f, 300.0f} + 100.0f*vec2{glm::cos(10.0f*t), glm::sin(10.0f*t)});
      return false;
    });
    gl::depth_test(false);
  }

  void on_load(shogle_state& state) {
    float t = 0.0f;
    cino_fumo = make_uptr<dynamic_model>(
      pool.get<render::model>("chiruno_fumo"),
      pool.get<render::shader>("generic_3d"),
      state.cam_3d
    );
    model::toggle_screen_space(*cino_fumo, true);
    model::move(*cino_fumo, {-0.35f, -0.25f, -1.0f});
    model::scale(*cino_fumo, vec3{0.015f});
    cino_fumo->add_task(make_uptr<fumo_jump>(PI*2.0f, 12.0f, 0.015f*0.5f));

    remu_fumo = make_uptr<dynamic_model>(
      pool.get<render::model>("reimu_fumo"),
      pool.get<render::shader>("generic_3d"),
      state.cam_3d
    );
    model::toggle_screen_space(*remu_fumo, true);
    model::move(*remu_fumo, {0.35f, -0.25f, -1.0f});
    model::scale(*remu_fumo, vec3{0.015f});
    remu_fumo->add_task(make_uptr<fumo_jump>(-PI*2.0f, 12.0f, 0.015f*0.5f));

    mari_fumo = make_uptr<dynamic_model>(
      pool.get<render::model>("marisa_fumo"),
      pool.get<render::shader>("generic_3d"),
      state.cam_3d
    );
    model::toggle_screen_space(*mari_fumo, true);
    model::move(*mari_fumo, {0.0f, -0.25f, -2.0f});
    model::scale(*mari_fumo, vec3{0.02f});
    model::rotate(*mari_fumo, {0.0f, -PI, 0.0f});
    mari_fumo->add_task([t](dynamic_model* obj, float dt) mutable {
      t += dt;
      model::rotate(*obj, {PI*2.0f*t, PI*3.0f*t, 0.0f});
      return false;
    });
    mari_fumo->add_task([t](dynamic_model* obj, float dt) mutable {
      float base_y = -0.25f;
      float force = 0.75f;
      float speed = 3.15f;
      t += dt;
      obj->_pos.y = base_y + (force*glm::abs(glm::sin(speed*t)));
      return false;
    });

    car = make_uptr<dynamic_model>(
      pool.get<render::model>("car"),
      pool.get<render::shader>("generic_3d"),
      state.cam_3d
    );
    model::toggle_screen_space(*car, true);
    model::move(*car, {0.0f, -0.25f, -2.0f});
    model::scale(*car, vec3{0.3f});
    model::rotate(*car, {0.0f, PI*0.5f, 0.0f});
    car->add_task(make_uptr<car_movement>(4.4f, 2.2f));

    loaded = true;
    gl::depth_test(true);
  }

  void update(shogle_state&, float dt) override {
    if (!loaded) {
      cino->update(dt);
    } else {
      cino_fumo->update(dt);
      remu_fumo->update(dt);
      mari_fumo->update(dt);
      car->update(dt);
    }
  }

  void draw(shogle_state&) override {
    if (!loaded) {
      cino->draw();
    } else {
      cino_fumo->draw();
      remu_fumo->draw();
      mari_fumo->draw();
      car->draw();
    }
  }

  static uptr<scene> create(shogle_state& state) { return make_uptr<test_async>(state); }
};

int main(int argc, char* argv[]) {
  Log::set_level(LogLevel::LOG_VERBOSE);

  std::string sett_path {SHOGLE_RESOURCES};
  sett_path += "/script/default_settings.lua";
  settings sett{argc, argv, sett_path.c_str()};

  auto shogle = shogle_create(sett.w_width, sett.w_height, sett.w_title);
  gl::blend(true);
  shogle_start(shogle, test_async::create);

  return EXIT_SUCCESS;
}
