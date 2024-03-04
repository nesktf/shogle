#include "scenes/test.hpp"
#include "core/logger.hpp"

namespace ntf::shogle {

class ChirunoSprite : public Sprite {
public:
  ChirunoSprite(const Texture& texture) : Sprite(texture) {
    this->pos_v = {400.0f, 300.0f, 0.0f};
    this->scale_v = {1.0f, 1.0f};
    this->rot = 0.0f;

    logger::info("[Chiruno] Initialized");
  }

  void update(float delta_time) override {
    this->rot += 10.0f*delta_time;
    this->update_model_m();
  }
};

class ChirunoModel : public Model {
public:
  ChirunoModel() {

  }
};

TestScene::TestScene() {
  this->setup_modeldata(ResourceMap {
    {"chiruno", "cirno_fumo"}
  });

  this->setup_texturedata(ResourceMap {
    {"chiruno_sprite", "cirno.png"}
  });

  this->setup_shaderdata(ResourceMap {
    {"chiruno_shader", "fbo"}
  });

  this->shader = new Shader();
  this->chiruno_tex = new Texture();
  this->chiruno_sprite = new ChirunoSprite(*chiruno_tex);
  this->chiruno_fumo = new ChirunoModel();

  logger::info("[TestScene] Initialized");
}

TestScene::~TestScene() {
  delete this->chiruno_fumo;
  delete this->chiruno_sprite;
  delete this->chiruno_tex;
  delete this->shader;

  logger::info("[TestScene] Terminated");
}

void TestScene::update(float delta_time) {
  this->chiruno_sprite->update(delta_time);
  this->chiruno_fumo->update(delta_time);
}

void TestScene::draw(void) {
  this->chiruno_sprite->draw(*shader);
  this->chiruno_fumo->draw(*shader);
}

}
