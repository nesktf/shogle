#include "shogle.hpp"

#include "render/glfw.hpp"
#include "event/input_handler.hpp"
#include "resource/loader.hpp"

#include "log.hpp"

namespace ntf::shogle {

bool Engine::init(const Settings& sett) {
  if (!window::init(sett.w_width, sett.w_height, sett.w_title.c_str())) {
    Log::error("[Engine] Failed to init window");
    return false;
  }

  InputHandler::instance().init();
  Log::verbose("[Engine] Input handler initialized");

  this->clear_color = sett.clear_color;
  this->should_close = false;
  this->last_frame = 0.0f;
  Log::verbose("[Engine] Settings applied");


  Log::info("[Engine] Initialized");
  return true;
}

Engine::~Engine() {
  window::destroy();
  Log::debug("[Engine] Terminated");
}

void Engine::start(LevelCreator creator) {
  Log::info("[Engine] Loading initial level");
  this->level = std::unique_ptr<Level>{creator()};
  Log::debug("[Engine] Initial level created");

  Log::info("[Engine] Entering main loop");
  glEnable(GL_DEPTH_TEST);
  while (!this->should_close) {
    InputHandler::instance().poll();
    res::DataLoader::instance().do_requests();

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
  Log::info("[Engine] Terminating program");
}

} // namespace ntf::shogle
