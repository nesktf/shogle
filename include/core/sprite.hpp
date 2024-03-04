#pragma once

#include "core/game_object.hpp"
#include "core/texture.hpp"

namespace ntf::shogle {

class Sprite : public GameObject {
public:
  Sprite(const Texture& texture) : texture(texture) {};

  void update_model_m(void);
  virtual void draw(Shader& shader) override;

public:
  std::reference_wrapper<const Texture> texture;

  glm::vec3 pos_v;
  glm::vec2 scale_v;
  float rot;
};

}
