#pragma once

#include "task/task.hpp"

namespace ntf::shogle {
template<typename TObj>
class GameObject;
}

namespace ntf::shogle::render {
class Model;
}

namespace ntf::shogle::task {

// using ModelTask = Task<GameObject<render::Model>>;
//
// ModelTask mod_transform(glm::vec3 pos, glm::vec3 scale, glm::vec3 rot);
// ModelTask mod_fumo_jump(float ang_speed, float jump_speed, float time);
// ModelTask mod_z_rot(float ang_speed, float time);
// ModelTask mod_linear_rel_move(glm::vec3 new_pos, float time);
// ModelTask mod_linear_abs_move(glm::vec3 new_pos, float time);
// ModelTask mod_funny_jump(float force, float time);

} // namsepace ntf::shogle
