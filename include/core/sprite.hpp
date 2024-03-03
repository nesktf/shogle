#pragma once

#include "core/game_object.hpp"
#include "core/texture.hpp"

namespace ntf::shogle {

class Sprite : public GameObject {
public:
  Sprite(const Texture& texture);
  void draw(Shader& shader) override;

public:
  std::reference_wrapper<const Texture> texture;
};

}
