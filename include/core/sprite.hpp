#pragma once

#include "core/game_object.hpp"
#include "core/texture.hpp"

namespace ntf::shogle {

class Sprite : public GameObject {
public:
  std::reference_wrapper<Texture> texture;

};

}
