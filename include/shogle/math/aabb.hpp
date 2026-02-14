#pragma once

#include "./common.hpp"

namespace shogle::math {

// clang-format off
template<typename T>
constexpr bool collision_aabb(const rectangle_pos<T>& a, const rectangle_pos<T>& b) {
  // AABB assumes pos is the lower left corner
  // Normalize scale, since obj.pos it's the quad's center
  const T a_width = a.width / T(2);
  const T a_height = a.height / T(2);
  const T b_width = a.width / T(2);
  const T b_height = a.height / T(2);
  const bool collision_x = a.x + a_width >= (b.x - b_width) && 
                           b.x + b_width >= (a.x - a_width);
  const bool collision_y = a.y + a_height >= (b.y - b_height) &&
                           b.y + b_height >= (a.y - a_height);

  return collision_x && collision_y;
}

// clang-format on

template<typename T>
constexpr bool collision_aabb(const rectangle_pos<T>& a, const circle_pos<T>& b) {
  // No need to normalize pos1 to be the center
  const T sq_rad = b.radius * b.radius;
  const T half_rect_w = a.width / T(2);
  const T half_rect_h = a.height / T(2);

  const T diff_x = b.x - a.x;
  const T diff_y = b.y - a.y;

  const T closest_x = (a.x + std::clamp(diff_x, -half_rect_w, half_rect_w)) - b.x;
  const T closest_y = (a.y + std::clamp(diff_y, -half_rect_h, half_rect_h)) - b.y;

  const T sq_len = (closest_x * closest_x) + (closest_y * closest_y);

  return sq_len < sq_rad;
}

template<typename T>
constexpr bool collision_aabb(const circle_pos<T>& a, const rectangle_pos<T>& b) {
  return collision_aabb(b, a);
}

template<typename T>
constexpr bool collision_aabb(const circle_pos<T>& a, const circle_pos<T>& b) {
  const T diff_x = b.x - a.x;
  const T diff_y = b.y - a.y;
  const T sq_sum = (a.radius + b.radius) * (a.radius + b.radius);
  const T sq_len = (diff_x * diff_x) + (diff_y * diff_y);
  return sq_len < sq_sum;
}

} // namespace shogle::math
