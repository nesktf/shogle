#pragma once

#include <core.hpp>
#include <version.hpp>

#include <math/alg.hpp>
#include <math/calc.hpp>
#include <math/collision.hpp>

#include <render/opengl/context.hpp>
#include <render/glfw/window.hpp>
#include <render/camera.hpp>
#include <render/meshes.hpp>
#include <render/uniform_tuple.hpp>
#include <render/render_loop.hpp>

#include <assets/atlas.hpp>
#include <assets/font.hpp>
#include <assets/model.hpp>
#include <assets/pool.hpp>
#include <assets/texture.hpp>

#include <scene/transform.hpp>
#include <scene/entity_pool.hpp>

#include <stl/allocator.hpp>
#include <stl/event.hpp>
#include <stl/expected.hpp>
#include <stl/logger.hpp>
#include <stl/optional.hpp>
#include <stl/singleton.hpp>
#include <stl/task.hpp>
#include <stl/threadpool.hpp>

#ifdef SHOGLE_ENABLE_IMGUI
#include <imgui.h>
#endif
