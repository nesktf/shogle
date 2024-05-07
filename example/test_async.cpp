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
        rotate(*obj, PI*0.5f*dt, {0.0f, 1.0f, 0.0f});

        obj->pos.x = 2.0f;
        this->state = State::GoingLeft;
        break;
      }
      case State::AtLeft: {
        rotate(*obj, -PI*0.5f*dt, {0.0f, 1.0f, 0.0f});

        obj->pos.x = -2.0f;
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
    obj->scale.y = _half_scale + (_half_scale*glm::abs(glm::sin(_jump_force*t)));

    set_rotation(*obj, {0.0f, _ang_speed*t, 0.0f});
  }

  float t {0.0f};
  float _ang_speed, _jump_force, _half_scale;
};

struct test_async : public scene {
  res::pool<render::shader, render::sprite, render::model> pool;

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
    pool.direct_request<render::sprite>({
      {.id="chiruno", .path="res/textures/cirno.png"}
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
      pool.get<render::sprite>("chiruno"),
      pool.get<render::shader>("generic_2d"),
      state.cam_2d
    );
    cino->toggle_screen_space(true);
    set_pos(*cino, {400.0f, 300.0f});
    set_scale(*cino, vec2{100.0f});

    cino->add_task([t](dynamic_sprite* obj, float dt) mutable {
      t += dt;
      rotate(*obj, PI*dt);
      return false;
    });
    cino->add_task([t](dynamic_sprite* obj, float dt) mutable {
      t += dt;
      set_pos(*obj, vec2{400.0f, 300.0f} + 100.0f*vec2{glm::cos(10.0f*t), glm::sin(10.0f*t)});
      return false;
    });
    gl::depth_test(false);
    state.input.subscribe(key::ESCAPE, key::PRESS, [&state]() {
      shogle_close_window(state);
    });
  }

  void on_load(shogle_state& state) {
    float t = 0.0f;
    cino_fumo = make_uptr<dynamic_model>(
      pool.get<render::model>("chiruno_fumo"),
      pool.get<render::shader>("generic_3d"),
      state.cam_3d
    );
    cino_fumo->toggle_screen_space(true);
    set_pos(*cino_fumo, {-0.35f, -0.25f, -1.0f});
    set_scale(*cino_fumo, vec3{0.015f});
    cino_fumo->add_task(make_uptr<fumo_jump>(PI*2.0f, 12.0f, 0.015f*0.5f));

    remu_fumo = make_uptr<dynamic_model>(
      pool.get<render::model>("reimu_fumo"),
      pool.get<render::shader>("generic_3d"),
      state.cam_3d
    );
    remu_fumo->toggle_screen_space(true);
    set_pos(*remu_fumo, {0.35f, -0.25f, -1.0f});
    set_scale(*remu_fumo, vec3{0.015f});
    remu_fumo->add_task(make_uptr<fumo_jump>(-PI*2.0f, 12.0f, 0.015f*0.5f));

    mari_fumo = make_uptr<dynamic_model>(
      pool.get<render::model>("marisa_fumo"),
      pool.get<render::shader>("generic_3d"),
      state.cam_3d
    );
    mari_fumo->toggle_screen_space(true);
    set_pos(*mari_fumo, {0.0f, -0.25f, -2.0f});
    set_scale(*mari_fumo, vec3{0.02f});
    set_rotation(*mari_fumo, {0.0f, -PI, 0.0f});
    mari_fumo->add_task([t](dynamic_model* obj, float dt) mutable {
      t += dt;
      set_rotation(*obj, {PI*2.0f*t, PI*3.0f*t, 0.0f});
      return false;
    });
    mari_fumo->add_task([t](dynamic_model* obj, float dt) mutable {
      float base_y = -0.25f;
      float force = 0.75f;
      float speed = 3.15f;
      t += dt;
      obj->pos.y = base_y + (force*glm::abs(glm::sin(speed*t)));
      return false;
    });

    car = make_uptr<dynamic_model>(
      pool.get<render::model>("car"),
      pool.get<render::shader>("generic_3d"),
      state.cam_3d
    );
    car->toggle_screen_space(true);
    set_pos(*car, {0.0f, -0.25f, -2.0f});
    set_scale(*car, vec3{0.3f});
    set_rotation(*car, {0.0f, PI*0.5f, 0.0f});
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

int main(void) {
  Log::set_level(LogLevel::LOG_VERBOSE);

  auto shogle = shogle_create(800, 600, shogle_gen_title("test_async"));
  shogle_main_loop(shogle, test_async::create);

  return EXIT_SUCCESS;
}

