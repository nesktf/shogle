#pragma once

#include "glad/glad.h"
#include <GLFW/glfw3.h>

namespace ntf::shogle {

GLFWwindow* glfw_init(unsigned int w_width, unsigned int w_height, const char* w_name);
void glfw_destroy(GLFWwindow* win);

}
