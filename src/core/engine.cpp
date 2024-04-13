#include "core/engine.hpp"
#include "core/input.hpp"
#include "core/log.hpp"
#include "core/error.hpp"

#include "res/resloader.hpp"

namespace ntf {

bool Shogle::init(const Settings& sett) {
  try {
    this->window = make_uptr<GLWindow>(sett.w_width, sett.w_height, sett.w_title.c_str());
  } catch(const ntf::error& e) {  
    Log::error("{}", e.what());
    return false;
  }
  this->clear_color = sett.clear_color;

  upd_proj2d_m((float)sett.w_width, (float)sett.w_height);
  upd_proj3d_m(window->ratio());
  upd_view_m();
  Log::verbose("[Shogle] Settings applied");

  window->set_fb_callback([](auto, int w, int h) {
    glViewport(0, 0, w, h); // 1,2 -> Location in window. 3,4 -> Size
    auto& eng = Shogle::instance();
    eng.upd_proj2d_m(w, h);
    eng.upd_proj3d_m((float)w/(float)h);
    Log::verbose("[Window] Viewport updated");
  });
  Log::verbose("[InputHandler] Framebuffer callback set");

  InputHandler::instance().init(window.get());

  Log::info("[Shogle] Initialized");
  return true;
}

void Shogle::start(SceneCreator creator) {
  this->level = creator();
  Log::verbose("[Shogle] Initial level created");

  Log::info("[Shogle] Entering main loop");
  while (!window->should_close()) {
    InputHandler::instance().poll();
    ResLoader::instance().do_requests();

    double curr_frame = window->get_time();
    double dt = curr_frame - this->last_frame;
    this->last_frame = curr_frame;

    glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    level->update(dt);

    window->swap_buffers();
  }
  Log::info("[Shogle] Terminating program");
}

} // namespace ntf
