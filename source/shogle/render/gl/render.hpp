#pragma once

#include <shogle/core/types.hpp>

#include <glad/glad.h>

namespace ntf::shogle::gl {

enum class clear : uint8_t {
  none = 0,
  depth = 1 << 0,
  stencil = 1 << 1,
};

constexpr inline clear operator|(clear a, clear b) {
  return static_cast<clear>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
constexpr inline clear operator&(clear a, clear b) {
  return static_cast<clear>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}
constexpr inline clear& operator|=(clear& a, clear b) {
  return a = static_cast<clear>(a | b);
}


enum class depth_fun {
  less = 0,
  lequal,
};

void init(GLADloadproc proc);
void terminate();

void set_viewport_size(vec2sz sz);

void clear_viewport(color4 color, clear flag = clear::none);
inline void clear_viewport(color3 color, clear flag = clear::none) {
  clear_viewport(color4{color, 1.0f}, flag);
}

void set_stencil_test(bool flag);
void set_depth_test(bool flag);
void set_blending(bool flag);

void set_depth_fun(depth_fun fun);

} // namespace ntf::gl
