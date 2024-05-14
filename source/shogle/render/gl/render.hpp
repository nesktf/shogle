#pragma once

#include <shogle/core/types.hpp>

#include <glad/glad.h>

namespace ntf::shogle::gl {

enum class depth_fun {
  less = 0,
  lequal,
};

void init(GLADloadproc proc);
void terminate();

void set_viewport_size(vec2sz sz);

void clear_viewport(vec4 color, bool depth, bool stencil);

void set_stencil_test(bool flag);
void set_depth_test(bool flag);
void set_blending(bool flag);

void set_depth_fun(depth_fun fun);

} // namespace ntf::gl
