#pragma once

namespace ntf::math {

// misc
template<typename TL, typename TR>
constexpr auto periodic_add(TL a, TR b, decltype(a+b) min, decltype(a+b) max) {
  auto res = a + b;
  auto range = max-min;
  while (res >= max) res -= range;
  while (res <  min) res += range;
  return res;
}

} // namespace ntf::math
