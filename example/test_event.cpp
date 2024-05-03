#include <shogle.hpp>

using namespace ntf;

struct test_event;

struct danmaku_spawner : public dynamic_sprite::task_t {
  danmaku_spawner(const ntf::Texture* tex, const ntf::Shader* sha, float ph, float fire_sp, float sp, unsigned int c);

  void update(test_event* obj, float dt) override;

  const ntf::Texture* danmaku_texture;
  const ntf::Shader* danmaku_shader;

  float t{0.0f};
  float phi{0.0f};

  float phase, f_speed, s_speed;
  unsigned int count;
};

struct chen_behaviour : public ntf::Task<ntf::Sprite> {
  enum class State {
    Init = 0,
    Moving,
    Idle,
    Shooting,
  };

  chen_behaviour(float _move_time, glm::vec2 _dest) :
    dest(_dest),
    move_time(_move_time) {}

  void update(ntf::Sprite* chen, float dt) override {
    t += dt;

    switch(chen_state) {
      case State::Init: {
        vel = (dest - chen->pos)/move_time;
        chen_state = State::Moving;
        break;
      }
      case State::Moving: {
        chen->pos += vel*dt;
        if (t >= move_time) {
          chen_event.fire(true);
          chen_state = State::Idle;
        }
        break;
      }
      case State::Idle: {
        if (start_blasting) {
          chen_state = State::Shooting;
        }
        break;
      }
      case State::Shooting: {
        chen->rot += 10.0f*dt;
        if (!start_blasting) {
          chen->rot = 0.0f;
          chen_state = State::Idle;
        }
        break;
      }
    }
  }

  float t{0.0f};
  bool start_blasting {false};
  State chen_state {State::Init};
  ntf::Event<bool> chen_event;

  glm::vec2 dest;
  float move_time;

  glm::vec2 vel{};
};

struct test_event : public ntf::TaskedScene<TestScene> {
  ntf::ResPool<ntf::Texture, ntf::Shader, ntf::ModelRes, ntf::Spritesheet> pool;

  std::vector<ntf::Sprite> new_danmaku;
  std::vector<std::pair<bool,ntf::Sprite>> danmaku;

  std::unique_ptr<ntf::Sprite> player;
  std::unique_ptr<ntf::Sprite> boss;
  std::unique_ptr<ntf::Sprite> chen_p;

  ntf::Event<bool>::Subscription chen_event_sub;

  test_event() {
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
    pool.direct_load<ntf::Spritesheet>({
      {
        .id="2hus",
        .path="_temp/2hus.json"
      },
      {
        .id="chen_p",
        .path="_temp/chen.json"
      }
    });

    float scale_f = 1.2f;

    player = std::make_unique<ntf::Sprite>(
      pool.get<ntf::Texture>("marisa"),
      pool.get<ntf::Shader>("generic_2d")
    );
    player->use_screen_space = true;
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


    chen_p = ntf::make_uptr<ntf::Sprite>(
      pool.get<ntf::Spritesheet>("chen_p"),
      "chen_p",
      pool.get<ntf::Shader>("generic_2d")
    );
    chen_p->color = {glm::vec3{1.0f}, 0.0f};
    chen_p->pos = {700.0f, 344.0f};
    chen_p->scale = {128.0f, 512.0f};
    chen_p->use_screen_space = true;

    boss = ntf::make_uptr<ntf::Sprite>(
      pool.get<ntf::Texture>("chen"),
      pool.get<ntf::Shader>("generic_2d")
    );
    boss->pos = {400.0f, -100.0f};
    boss->scale = scale_f*glm::vec2{48.0f, 64.0f};
    boss->use_screen_space = true;

    auto* chen_b = ntf::make_ptr<chen_behaviour>(1.0f, glm::vec2{400.0f, 300.0f});
    auto sub_event = [chen_b,this](...) {
      float t = 0.0f;
      float app_t = 0.25f;
      chen_p->add_task([chen_b,this,t,app_t](ntf::Sprite* obj, float dt) mutable {
        t += dt;

        obj->color.w += dt/app_t;
        obj->pos.x += 100.0f*dt;
        obj->scale.x += 100.0*dt;
        obj->scale.y += 4*100.0*dt;

        if (t >= app_t) {
          auto& in = ntf::InputHandler::instance();
          t = 0.0f;
          in.register_listener(ntf::KEY_J, ntf::KEY_PRESS, [t, app_t,this, chen_b]() {
            auto& in = ntf::InputHandler::instance();
            chen_p->set_index(1);
            chen_p->add_task([t, app_t](ntf::Sprite* obj, float dt) mutable {
              t+=dt;
              obj->pos.x -= 100.0f*dt;
              obj->color.w -= dt/app_t;
              obj->scale.x -= 100.0*dt;
              obj->scale.y -= 4*100.0*dt;

              return (t >= app_t);
            });
            chen_b->start_blasting = true;
            this->add_task(ntf::make_uptr<danmaku_spawner>(pool.get<ntf::Texture>("danmaku"), pool.get<ntf::Shader>("generic_2d"),M_2_PIf, 0.05f, 200.0f, 8u));

            in.unregister_all();
          });
          return true;
        }

        return false;
      });
    };


    chen_event_sub = chen_b->chen_event.subscribe(sub_event);
    boss->add_task(chen_b);

    player->add_task([this, chen_b](auto* obj, float) -> bool {
      for (auto& d : danmaku) {
        if (ntf::collision2d(d.second.pos, 16.0f, obj->pos, 4.0f)) {
          ntf::Log::debug("PICHUUUUN");
          chen_b->start_blasting = false;
          this->end_tasks();
          danmaku.clear();
          float t = 0.0f;
          float app_t = 0.25f;
          chen_p->set_index(2);
          chen_p->add_task([t, app_t](auto* o, float dt) mutable {
            t+=dt;
            o->color.w += dt/app_t;
            o->pos.x += 100.0f*dt;
            o->scale.x += 100.0*dt;
            o->scale.y += 4*100.0*dt;
            return (t>=app_t);
          });
          break;
        }
      }
      return false;
    });
  }

  void update(float dt) override {
    for (auto& d : new_danmaku) {
      danmaku.push_back(std::make_pair(false,std::move(d)));
    }
    new_danmaku.clear();

    this->do_tasks(this, dt);

    boss->update(dt);
    boss->draw();

    player->update(dt);
    player->draw();

    chen_p->update(dt);
    chen_p->draw();

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

danmaku_spawner::danmaku_spawner(const ntf::Texture* tex, const ntf::Shader* sha, float ph, float fire_sp, float sp, unsigned int c):
    danmaku_texture(tex),
    danmaku_shader(sha),
    phase(ph),
    f_speed(fire_sp),
    s_speed(sp),
    count(c){}

void danmaku_spawner::update(test_event* scene, float dt) {
  t += dt;
  phi += phase*dt;

  if (t > f_speed) {
    t = 0.0f;
    for (float i = 0; i < (float)count; i += 1.0f) {
      auto bullet = ntf::Sprite{danmaku_texture, danmaku_shader};
      bullet.pos = scene->boss->pos;
      bullet.scale = {16.0f, 16.0f};
      bullet.use_screen_space = true;
      float x = i*(M_PI/((float)count*0.5f)) + phi;
      glm::vec2 speed = s_speed*glm::vec2{glm::cos(x), glm::sin(x)};
      bullet.color = {glm::vec3{1.0f}, 1.0f};
      float t_ = 0.0f;
      bullet.add_task([t_,speed](ntf::Sprite* danmaku, float dt2) mutable {
        t_ += dt2;
        danmaku->pos += speed*dt2;
        danmaku->color.r = glm::abs(glm::sin(5.0f*t_));
        danmaku->color.g = glm::abs(glm::sin(5.0f*t_));
        danmaku->color.b = glm::abs(glm::sin(5.0f*t_));
        return false;
      });
      scene->new_danmaku.push_back(std::move(bullet));
    }
  }
}

int main(int argc, char* argv[]) {
  ntf::Log::set_level(ntf::LogLevel::LOG_VERBOSE);
  std::string sett_path {SHOGLE_RESOURCES};
  sett_path += "/script/default_settings.lua";

  auto& shogle = ntf::Shogle::instance();
  if (shogle.init(ntf::Settings{argc, argv, sett_path.c_str()})) {
    shogle.enable_depth_test(false);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    shogle.start(TestScene::create);
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

