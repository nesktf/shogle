#pragma once

#include "glad/glad.h"
#include <GLFW/glfw3.h>

namespace ntf::shogle {

GLFWwindow* glfw_init(size_t w_width, size_t w_height, const char* w_name);
void glfw_destroy(GLFWwindow* win);

}
