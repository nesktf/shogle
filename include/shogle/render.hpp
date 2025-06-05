#pragma once

#if defined(SHOGLE_EXPOSE_GLFW) && SHOGLE_EXPOSE_GLFW
#include <glad/glad.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

#include "../../src/core.hpp"

#include "../../src/render/buffer.hpp"
#include "../../src/render/texture.hpp"
#include "../../src/render/pipeline.hpp"
#include "../../src/render/shader.hpp"
#include "../../src/render/framebuffer.hpp"
#include "../../src/render/vertex.hpp"

#include "../../src/render/window.hpp"

#if defined(SHOGLE_ENABLE_IMGUI) && SHOGLE_ENABLE_IMGUI
#if defined(SHOGLE_EXPOSE_GLFW) && SHOGLE_EXPOSE_GLFW
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#endif
#include "../../src/render/imgui.hpp"
#endif
