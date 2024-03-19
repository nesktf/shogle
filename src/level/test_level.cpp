#include "level/test_level.hpp"
#include "render/sprite.hpp"
#include "render/model.hpp"

namespace ntf::shogle {

class ChirunoSprite : public GameObject {
public:
  ChirunoSprite(res::Texture* texture, res::Shader* shader) {
    this->obj = new Sprite(texture, shader);
    this->pos = glm::vec2{400.0f, 300.0f};
    this->scale = glm::vec2{1.0f, 1.0f};
    Log::verbose("[ChirunoSprite] Initialized");
  }
  ~ChirunoSprite() {
    delete this->obj;
    Log::verbose("[ChirunoSprite] Deleted sprite");
  }

public:
  void update(float dt) override {
    this->obj->model_m = gen_model_2d(pos, scale, 10.0f*time_elapsed);
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
    this->pos = glm::vec3{0.0f, 0.0f, -1.0f};
    this->base_scale = 0.02f;
    this->scale = glm::vec3{base_scale};
    this->jump_speed = 5.0f;
    this->ang_speed = 10.0f;
    Log::verbose("[ChirunoFumo] Initialized");
  }
  ~ChirunoFumo() {
    delete this->obj;
    Log::verbose("[ChirunoFumo] Deleted model");
  }

public:
  void update(float dt) override {
    float half_scale = base_scale / 2.0f;
    this->scale.y = (half_scale)*glm::abs(half_scale*glm::sin(jump_speed*time_elapsed));
    this->rot.y = ang_speed * time_elapsed;

    this->obj->model_m = gen_model_3d(pos, scale, rot);
    time_elapsed += dt;
  }

public:
  glm::vec3 pos, scale, rot;
  float time_elapsed, base_scale, ang_speed, jump_speed;
};

TestLevel::TestLevel() {
  auto& loader {res::ResLoader::instance()};

  init_tex.add_request(res::ResPath{
    .id = "chiruno",
    .path = "res/textures/cirno.png"
  });

  shaders.add_request(res::ResPath{
    .id = "generic_3d",
    .path = "res/shaders/generic_3d"
  });

  shaders.add_request(res::ResPath{
    .id = "generic_2d",
    .path = "res/shaders/generic_2d"
  });

  loader.request(init_tex);
  loader.request(shaders);

  auto* texture = init_tex.get_p("chiruno");
  auto* shader = shaders.get_p("generic_2d");

  auto* cino = new ChirunoSprite(texture, shader);
  objs.emplace(std::make_pair("chiruno", std::unique_ptr<GameObject>{cino}));

  models.load_callback = [this]{
    next_state();
  };
  models.add_request(res::ResPath{
    .id = "chiruno_fumo",
    .path = "res/models/cirno_fumo/cirno_fumo.obj"
  });
  loader.async_request(models);
}

void TestLevel::on_load(void) {
  auto* shader = shaders.get_p("generic_3d");
  auto* model = models.get_p("chiruno_fumo");
  auto* cino_fumo = new ChirunoFumo(model, shader);
  objs.emplace(std::make_pair("chiruno_fumo", std::unique_ptr<GameObject>{cino_fumo}));

  objs["chiruno"]->enabled = false;
}

void TestLevel::update_loading(float dt) {
  objs["chiruno"]->update(dt);
}

void TestLevel::update_loaded(float dt) {
  objs["chiruno_fumo"]->update(dt);
}


} // namespace ntf::shogle
