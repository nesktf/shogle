#pragma once

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <functional>

namespace ntf::shogle::window {

bool init(size_t w_width, size_t w_height, const char* w_name);
void destroy(void);
void set_close(void);
void swap_buffer(void);
bool should_close(void);
void set_fb_callback(GLFWframebuffersizefun callback);

}
