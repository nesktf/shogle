#include <shogle/core/engine.hpp>

#include <shogle/core/error.hpp>
#include <shogle/core/log.hpp>

#include <shogle/render/imgui.hpp>

#include <shogle/res/loader.hpp>

namespace ntf {

using namespace render;

static void update_cameras(camera2D& cam_2d, camera3D& cam_3d, size_t w, size_t h);

shogle_state::shogle_state(glfw::window window, size_t w, size_t h) :
  win(window), input(win) { update_cameras(cam_2d, cam_3d, w, h); }

shogle_state::~shogle_state() {
  glfw::destroy_window(win);
}

shogle_state shogle_create(size_t window_width, size_t window_height, std::string window_title) {
  glfw::window win;

  try {
    win = glfw::create_window(window_width, window_height, window_title);
  } catch(const ntf::error& err) {
    Log::error("{}", err.what());
    throw;
  }

  return shogle_state{std::move(win), window_width, window_height};
}

void shogle_start(shogle_state& state, scene_creator_t creator) {
  glfw::set_user_ptr(state.win, &state);

  glfw::set_viewport_callback(state.win, [](GLFWwindow* win, int w, int h) {
    auto* state = static_cast<shogle_state*>(glfw::get_user_ptr(win));
    render::gl::set_viewport(w, h);
    update_cameras(state->cam_2d, state->cam_3d, w, h);
    state->viewport.fire(w, h);
  });

  glfw::set_key_callback(state.win, [](GLFWwindow* win, int key, int, int action, int) {
    auto state = static_cast<shogle_state*>(glfw::get_user_ptr(win));
    state->input.fire(static_cast<glfw::key>(key), static_cast<glfw::key_action>(action));
  });

  imgui::init(state.win);

  double last_frame {0.0};

  auto scene = creator(state);
  scene->on_create(state);

  while (glfw::is_window_open(state.win)) {
    state.loader.do_requests();

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

  state.input.clear();
  state.viewport.clear();
}

void shogle_close_window(shogle_state& state) {
  glfw::set_close(state.win, true);
}

static void update_cameras(camera2D& cam_2d, camera3D& cam_3d, size_t w, size_t h) {
  cam_2d.set_viewport({static_cast<float>(w), static_cast<float>(h)});
  cam_2d.update({});

  cam_3d.set_viewport({static_cast<float>(w), static_cast<float>(h)});
  cam_3d.update({});
}


} // namespace ntf
