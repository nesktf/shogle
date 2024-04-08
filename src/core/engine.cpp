#include "engine.hpp"

#include "input.hpp"
#include "resource/loader.hpp"

#include "log.hpp"

namespace ntf::shogle {

bool Engine::init(const Settings& sett) {
  try {
    this->window = GLWindow::create(sett.w_width, sett.w_height, sett.w_title.c_str());
  } catch(const shogle_error& e) {  
    Log::error("{}", e.what());
    return false;
  }
  this->clear_color = sett.clear_color;
  this->should_close = false;
  this->last_frame = 0.0f;
  upd_proj2d_m((float)sett.w_width, (float)sett.w_height);
  upd_proj3d_m(window->ratio());
  upd_view_m();
  Log::verbose("[Engine] Settings applied");

  window->set_fb_callback([](auto, int w, int h) {
    glViewport(0, 0, w, h); // 1,2 -> Location in window. 3,4 -> Size
    auto& eng = Engine::instance();
    eng.upd_proj2d_m(w, h);
    eng.upd_proj3d_m((float)w/(float)h);
    Log::verbose("[Window] Viewport updated");
  });
  Log::verbose("[InputHandler] Framebuffer callback set");

  InputHandler::instance().init(window.get());

  Log::info("[Engine] Initialized");
  return true;
}

void Engine::start(LevelCreator creator) {
  this->level = std::unique_ptr<Scene>{creator()};
  Log::verbose("[Engine] Initial level created");

  Log::info("[Engine] Entering main loop");
  glEnable(GL_DEPTH_TEST);
  while (!window->should_close()) {
    InputHandler::instance().poll();
    res::ResourceLoader::instance().do_requests();

    double curr_frame = window->get_time();
    double dt = curr_frame - this->last_frame;
    this->last_frame = curr_frame;

    glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    level->update(dt);

    window->swap_buffers();
  }
  Log::info("[Engine] Terminating program");
}

} // namespace ntf::shogle
