#include <shogle/core/engine.hpp>

#include <shogle/core/error.hpp>
#include <shogle/core/log.hpp>

#include <shogle/render/imgui.hpp>

#include <shogle/res/loader.hpp>

namespace ntf {

using imgui = render::imgui;
using glfw = render::glfw;

static void update_cameras(camera2D& cam_2d, camera3D& cam_3d, size_t w, size_t h);

shogle_state::shogle_state(glfw::window window, size_t w, size_t h) :
  win(window), input(win) { 
    update_cameras(cam_2d, cam_3d, w, h); 
    Log::debug("[shogle] State initialized");
    Log::info("[shogle] Initialized"); 
}

shogle_state::~shogle_state() {
  glfw::destroy_window(win);
  Log::info("[shogle] Terminated");
}

shogle_state shogle_create(size_t window_width, size_t window_height, std::string window_title) {
  Log::debug("[shogle] Initializing shogle state");
  glfw::window win;

  try {
    win = glfw::create_window(window_width, window_height, window_title);
  } catch(const ntf::error& err) {
    Log::error("{}", err.what());
    throw;
  }

  return shogle_state{std::move(win), window_width, window_height};
}

void shogle_main_loop(shogle_state& state, scene_creator_t creator) {
  Log::debug("[shogle] Creating scene");
  glfw::set_user_ptr(state.win, &state);

  glfw::set_viewport_callback(state.win, [](GLFWwindow* win, int w, int h) {
    Log::debug("[shogle] Viewport event: {} {}", w, h);
    auto* state = static_cast<shogle_state*>(glfw::get_user_ptr(win));
    render::gl::set_viewport(w, h);
    if (state->update_camera_viewport) {
      update_cameras(state->cam_2d, state->cam_3d, w, h);
    }
    state->viewport.fire(w, h);
  });
  Log::verbose("[shogle] GLFW viewport callback set");

  glfw::set_key_callback(state.win, [](GLFWwindow* win, int key, int, int action, int) {
    // Log::verbose("[shogle] Key event: {} {}", key, action);
    auto state = static_cast<shogle_state*>(glfw::get_user_ptr(win));
    state->input.fire(static_cast<glfw::key>(key), static_cast<glfw::key_action>(action));
  });
  Log::verbose("[shogle] GLFW key callback set");

  imgui::init(state.win);
  Log::debug("[shogle] imgui initialized");

  gl::blend(true); // temp?

  double last_frame {0.0};

  auto scene = creator();
  scene->on_create(state);
  Log::debug("[shogle] Scene created");

  Log::info("[shogle] Main loop start");
  while (glfw::is_window_open(state.win)) {
    imgui::new_frame();
    glfw::new_frame(state.win);

    double curr_frame = glfw::elapsed_time(state.win);
    double dt = curr_frame - last_frame;
    last_frame = curr_frame;

    gl::clear_viewport();
    scene->update(state, dt);
    scene->draw(state);

    imgui::render();
    glfw::end_frame(state.win);
  }
  Log::info("[shogle] Main loop exit");

  imgui::destroy();
  Log::debug("[shogle] imgui destroyed");

  state.input.clear();
  Log::verbose("[shogle] Cleared input events");
  state.viewport.clear();
  Log::verbose("[shogle] Cleared viewport events");
}

void shogle_close_window(shogle_state& state) {
  glfw::set_close(state.win, true);
  Log::debug("[shogle] Window closed");
}

static void update_cameras(camera2D& cam_2d, camera3D& cam_3d, size_t w, size_t h) {
  cam_2d.set_viewport({static_cast<float>(w), static_cast<float>(h)});
  cam_2d.update({});
  Log::verbose("[shogle] default 2d camera updated");

  cam_3d.set_viewport({static_cast<float>(w), static_cast<float>(h)});
  cam_3d.update({});
  Log::verbose("[shogle] default 3d camera updated");
}


} // namespace ntf
