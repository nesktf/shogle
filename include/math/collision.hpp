#pragma once

#include <glm/vec2.hpp>

namespace ntf {

bool collision2d(glm::vec2 pos1, glm::vec2 size1, glm::vec2 pos2, glm::vec2 size2);
bool collision2d(glm::vec2 pos1, glm::vec2 size1, glm::vec2 pos2, float rad2);
bool collision2d(glm::vec2 pos1, float rad1, glm::vec2 pos2, glm::vec2 size2);
bool collision2d(glm::vec2 pos1, float rad1, glm::vec2 pos2, float rad2);

} // namespace ntf
