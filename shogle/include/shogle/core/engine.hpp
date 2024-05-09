#pragma once

#include <shogle/core/event.hpp>
#include <shogle/core/input.hpp>

#include <shogle/res/loader.hpp>

#include <shogle/scene/camera.hpp>

namespace ntf {

using key = render::glfw; // for convenience
using gl = render::gl;

using viewport_event = event<size_t, size_t>;

inline std::string shogle_gen_title(const char* title) {
  return std::string{title}+" - shogle v"+std::string{SHOGLE_VERSION};
}

inline std::string shogle_res_path(void) {
  return std::string{SHOGLE_RESOURCES};
}

struct shogle_state {
  using glfw = render::glfw;

  shogle_state(glfw::window window, size_t w, size_t h);
  ~shogle_state();

  glfw::window win;

  input_event input;
  viewport_event viewport;

  camera2D cam_2d;
  camera3D cam_3d;
  bool update_camera_viewport {false};
};

shogle_state shogle_create(size_t window_width, size_t window_height, std::string window_title);
void shogle_close_window(shogle_state& state);
void shogle_main_loop(shogle_state& state, scene_creator_t creator);


} // namespace ntf
