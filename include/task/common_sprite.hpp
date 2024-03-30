#pragma once

#include "task/common_sprite.hpp"
#include "level/game_object.hpp"

namespace ntf::shogle::render {
class Sprite;
}

namespace ntf::shogle::task {

using SpriteTask = Task<GameObject<render::Sprite>>;

SpriteTask spr_init_transform(glm::vec2 pos, glm::vec2 scale, float rot);
SpriteTask spr_rot_right(float ang_speed, float time);
SpriteTask spr_rot_left(float ang_speed, float time);

} //namespace ntf::shogle::task
