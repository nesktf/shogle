#include "core/game_state.hpp"
// #include "scenes/test.hpp"
#include "core/logger.hpp"
#include "core/event_handler.hpp"
#include "core/renderer.hpp"

namespace ntf::shogle {

void GameState::init(size_t w_width, size_t w_height, const char* w_name, int argc, char* argv[]) {
  if (!window::init(w_width, w_height, w_name)) {
    logger::fatal("[GameState] Failed to init window");
  }

  EventHandler::instance().init();
  Renderer::instance().update_proj_m(w_width, w_height);

  this->clear_color = {0.2f, 0.2f, 0.2f};
  this->scene = new TestScene();


  logger::debug("[GameState] Initialized");
}

GameState::~GameState() {
  delete this->scene;

  window::destroy();

  logger::debug("[GameState] Terminated");
}


void GameState::exit_loop(void) {
  window::set_close();
  logger::debug("[GameState] Exit triggered");
}

bool GameState::main_loop(void) {
  EventHandler::instance().poll();

  ResourceLoader::instance().do_requests();

  static float last_frame = 0.0f;
  float curr_frame = glfwGetTime();
  float dt = curr_frame - last_frame;
  last_frame = curr_frame;

  scene->update(dt);

  glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  scene->draw();

  window::swap_buffer();
  return !window::should_close();
}

}
