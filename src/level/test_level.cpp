#include "level/test_level.hpp"
#include "render/sprite.hpp"
#include "render/model.hpp"

namespace ntf::shogle {

class ChirunoSprite : public GameObject {
public:
  ChirunoSprite(res::Texture* texture, res::Shader* shader) {
    this->obj = new Sprite(texture, shader);
    this->pos = glm::vec2{400.0f, 300.0f};
    this->scale = glm::vec2{400.0f, 400.0f};
    Log::verbose("[ChirunoSprite] Initialized");
  }
  ~ChirunoSprite() {
    delete this->obj;
    Log::verbose("[ChirunoSprite] Deleted sprite");
  }

public:
  void update(float dt) override {
    this->obj->model_m = gen_model_2d(pos, scale, 100.0f*time_elapsed);
    time_elapsed += dt;
  }

public:
  glm::vec2 pos, scale;
  float time_elapsed = 0.0f;
};

class ChirunoFumo : public GameObject {
public:
  ChirunoFumo(res::Model* model, res::Shader* shader) {
    this->obj = new Model(model, shader);
    this->pos = glm::vec3{0.0f, -0.25f, -1.0f};
    this->base_scale = 0.015f;
    this->scale = glm::vec3{base_scale};
    this->rot = glm::vec3{0.0f};
    this->jump_speed = 8.0f;
    this->ang_speed = 200.0f;
    Log::verbose("[ChirunoFumo] Initialized");
  }
  ~ChirunoFumo() {
    delete this->obj;
    Log::verbose("[ChirunoFumo] Deleted model");
  }

public:
  void update(float dt) override {
    float half_scale = base_scale / 2.0f;
    this->scale.y = half_scale + (half_scale*glm::abs(glm::sin(jump_speed*time_elapsed)));
    this->rot.y += ang_speed * dt;

    this->obj->model_m = gen_model_3d(pos, scale, rot);
    time_elapsed += dt;
  }

public:
  glm::vec3 pos, scale, rot;
  float time_elapsed {0.0f}, base_scale, ang_speed, jump_speed;
};

TestLevel::TestLevel() {
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

  auto* cino = new ChirunoSprite(
    pool.get_p<res::Texture>("chiruno"),
    pool.get_p<res::Shader>("generic_2d")
  );
  objs.emplace(std::make_pair("chiruno", std::unique_ptr<GameObject>{cino}));
}

void TestLevel::on_load(void) {
  auto* cino_fumo = new ChirunoFumo(
    pool.get_p<res::Model>("chiruno_fumo"),
    pool.get_p<res::Shader>("generic_3d")
  );
  objs.emplace(std::make_pair("chiruno_fumo", std::unique_ptr<GameObject>{cino_fumo}));

  auto* cino = dynamic_cast<ChirunoSprite*>(objs["chiruno"].get());
  cino->pos = {100.0f, 100.0f};
  cino->scale = cino->scale/2.0f;

  auto* cino2 = new ChirunoSprite(
    pool.get_p<res::Texture>("chiruno"),
    pool.get_p<res::Shader>("generic_2d")
  );
  cino2->pos = {700.0f, 100.0f};
  cino2->scale = cino2->scale/2.0f;
  objs.emplace(std::make_pair("chiruno2", std::unique_ptr<GameObject>{cino2}));
}

void TestLevel::update_loading(float dt) {
  for (auto& obj : objs) {
    obj.second->update(dt);
  }
}

void TestLevel::update_loaded(float dt) {
  update_loading(dt);
}


} // namespace ntf::shogle
