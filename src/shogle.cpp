#include "shogle.hpp"

#include "render/glfw.hpp"
#include "event/input_handler.hpp"
#include "resource/resource.hpp"

#include "log.hpp"

namespace ntf::shogle {

bool Engine::init(const Settings& sett, LevelCreator creator) {
  if (!window::init(sett.w_width, sett.w_height, sett.w_title.c_str())) {
    log::error("[Engine] Failed to init window");
    return false;
  }

  InputHandler::instance().init();
  this->clear_color = sett.clear_color;
  this->should_close = false;
  this->last_frame = 0.0f;

  this->level = std::unique_ptr<Level>{creator()};

  log::debug("[Engine] Initialized");
  return true;
}

Engine::~Engine() {
  window::destroy();
  log::debug("[Engine] Terminated");
}

void Engine::start(void) {
  while (!this->should_close) {
    InputHandler::instance().poll();
    res::ResLoader::instance().do_requests();

    float curr_frame = glfwGetTime();
    float dt = curr_frame - this->last_frame;
    this->last_frame = curr_frame;

    level->update(dt);

    glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    level->draw();

    window::swap_buffer();
    this->should_close = window::should_close();
  }
}

} // namespace ntf::shogle
