#include "math/collision.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace ntf {

bool collision2d(glm::vec2 pos1, glm::vec2 size1, glm::vec2 pos2, glm::vec2 size2) {
  // AABB assumes pos is the lower left corner
  // Normalize scale, since obj.pos it's the quad's center
  glm::vec2 normsize1 = size1 / 2.0f;
  glm::vec2 normsize2 = size2 / 2.0f;

  bool col_x = 
    pos1.x + normsize1.x >= (pos2.x - normsize2.x) &&
    pos2.x + normsize2.x >= (pos1.x - normsize1.x);
  bool col_y = 
    pos1.y + normsize1.y >= (pos2.y - normsize2.y) &&
    pos2.y + normsize2.y >= (pos1.y - normsize1.y);

  return col_x && col_y;
}

bool collision2d(glm::vec2 pos1, glm::vec2 size1, glm::vec2 pos2, float rad2) {
  // No need to normalize pos1 to be the center
  float sq_rad = rad2*rad2; // mult is cheaper than sqrt?
  glm::vec2 half_rect = size1 / 2.0f;

  glm::vec2 diff = pos2 - pos1;
  glm::vec2 diff_clamp = glm::clamp(diff, -half_rect, half_rect);
  glm::vec2 closest_p = pos1 + diff_clamp;

  diff = closest_p - pos2;
  float sq_len = (diff.x*diff.x) + (diff.y*diff.y);

  return sq_len < sq_rad;
}

bool collision2d(glm::vec2 pos1, float rad1, glm::vec2 pos2, glm::vec2 size2) {
  return collision2d(pos2, size2, pos1, rad1);
}

bool collision2d(glm::vec2 pos1, float rad1, glm::vec2 pos2, float rad2) {
  glm::vec2 diff = pos2 - pos1;

  float sq_sum = (rad1+rad2)*(rad1+rad2); // maybe it is
  float sq_len = (diff.x*diff.x) + (diff.y*diff.y);

  return sq_len < sq_sum;
}

} // namespace ntf
