#pragma once

#include "core/game_object.hpp"
#include "core/texture.hpp"

namespace ntf::shogle {

class SpriteObject : public GameObject {
public:
  SpriteObject(const Texture& texture);
  ~SpriteObject() = default;

  SpriteObject(SpriteObject&&) = default;
  SpriteObject& operator=(SpriteObject&&) = default;

  SpriteObject(const SpriteObject&) = delete;
  SpriteObject& operator=(const SpriteObject&) = delete;

public:
  virtual void update(float dt) override;
  void draw(Shader& shader) const override;

public:
  std::reference_wrapper<const Texture> texture;
  glm::vec3 pos_v;
  glm::vec2 scale_v;
  float rot;
};

}
