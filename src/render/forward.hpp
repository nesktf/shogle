#pragma once

#include "../math/matrix.hpp"

#include "../stl/expected.hpp"

#include "../stl/ptr.hpp"

#include <glad/glad.h>

#if SHOGLE_ENABLE_IMGUI
#include <imgui.h>
#include <imgui_impl_opengl3.h> // Should work fine with OGL 4.x
#endif

#if SHOGLE_USE_GLFW
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#if SHOGLE_ENABLE_IMGUI
#include <imgui_impl_glfw.h>
#endif
#endif

#define SHOGLE_DECLARE_RENDER_HANDLE(_name) \
class _name { \
public: \
  constexpr _name() : _handle{r_handle_tombstone} {} \
  constexpr explicit _name(r_handle_value handle) : _handle{handle} {} \
public: \
  constexpr explicit operator r_handle_value() const noexcept { return _handle; } \
  constexpr bool valid() const noexcept { return _handle != r_handle_tombstone; } \
  constexpr explicit operator bool() const noexcept { return valid(); } \
  constexpr bool operator==(const _name& rhs) const noexcept { \
    return _handle == static_cast<r_handle_value>(rhs); \
  } \
private: \
  r_handle_value _handle; \
}; \

namespace ntf {

using color3 = vec3;
using color4 = vec4;

enum class r_api : uint8 {
  software,
  opengl,
  vulkan,
};
class r_context;
class r_context_view;

enum class r_win_api : uint8 {
  glfw,
  sdl2, // ?
};
class r_window;

using r_handle_value = uint32;
constexpr r_handle_value r_handle_tombstone = std::numeric_limits<r_handle_value>::max();

using r_error = ::ntf::error<void>;

template<typename T>
using r_expected = ::ntf::expected<T, r_error>;

template<typename T>
struct r_handle_hash {
  r_handle_hash() noexcept = default;
  size_t operator()(const T& handle) const noexcept {
    return std::hash<r_handle_value>{}(static_cast<r_handle_value>(handle));
  }
};

} // namespace ntf

// #undef SHOGLE_DECLARE_RENDER_HANDLE
