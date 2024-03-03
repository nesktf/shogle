#include "core/game_state.hpp"
#include "scenes/test.hpp"
#include "core/logger.hpp"

namespace ntf::shogle {

void GameState::init(size_t w_width, size_t w_height, const char* w_name, int argc, char* argv[]) {
  this->window = glfw_init(w_width, w_height, w_name);
  if (!this->window) {
    logger::fatal("[GameState] Failed to init window");
  }
  this->clear_color = { 0.2f, 0.2f, 0.2f };
  this->scene = new TestScene();

  logger::debug("[GameState] Initialized");
}

GameState::~GameState() {
  delete this->scene;

  if (this->window) {
    glfw_destroy(this->window);
  }

  logger::debug("[GameState] Terminated");
}


void GameState::terminate(void) {
  glfwSetWindowShouldClose(this->window, true);
}

bool GameState::main_loop(void) {
  float curr_frame = glfwGetTime();
  float dt = curr_frame - this->last_frame;
  this->last_frame = curr_frame;

  glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  scene->draw(Renderer::instance());
  scene->update(dt);

  glfwSwapBuffers(this->window);
  glfwPollEvents();
  return !glfwWindowShouldClose(this->window);
}

}
