#pragma once

#include <shogle/core/types.hpp>

namespace ntf::math {

bool collision2d(vec2 pos1, vec2 size1, vec2 pos2, vec2 size2);
bool collision2d(vec2 pos1, vec2 size1, vec2 pos2, float rad2);
bool collision2d(vec2 pos1, float rad1, vec2 pos2, vec2 size2);
bool collision2d(vec2 pos1, float rad1, vec2 pos2, float rad2);

} // namespace ntf::math
