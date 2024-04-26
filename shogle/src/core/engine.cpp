#include <shogle/core/engine.hpp>
#include <shogle/core/input.hpp>
#include <shogle/core/log.hpp>
#include <shogle/core/error.hpp>

#include <shogle/res/resloader.hpp>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

namespace ntf {

bool Shogle::init(const Settings& sett) {
  try {
    _window = make_uptr<GLWindow>(sett.w_width, sett.w_height, sett.w_title.c_str());
  } catch(const ntf::error& e) {  
    Log::error("{}", e.what());
    return false;
  }
  clear_color = sett.clear_color;


  cam2D_default = Camera2D{Camera2D::proj_info{
    .viewport = {static_cast<float>(sett.w_width), static_cast<float>(sett.w_height)},
    .layer_count = 10
  }};
  cam3D_default = Camera3D{Camera3D::proj_info{
    .viewport = {static_cast<float>(sett.w_width), static_cast<float>(sett.w_height)},
    .draw_dist = {0.1f, 100.0f},
    .fov = M_PIf*0.25f
  }};
  Log::verbose("[Shogle] Settings applied");

  _window->set_fb_callback([](auto, int w, int h) {
    glViewport(0, 0, w, h); // 1,2 -> Location in window. 3,4 -> Size
    auto& eng = Shogle::instance();
    eng.cam2D_default.set_viewport({static_cast<float>(w), static_cast<float>(h)});
    eng.cam3D_default.set_viewport({static_cast<float>(w), static_cast<float>(h)});
    Log::verbose("[Window] Viewport updated");
  });
  Log::verbose("[InputHandler] Framebuffer callback set");

  InputHandler::instance().init(_window.get());

  Log::info("[Shogle] Initialized");
  return true;
}

void Shogle::start(SceneCreator creator) {
  _scene = creator();
  Log::verbose("[Shogle] Initial level created");

  Log::info("[Shogle] Entering main loop");
  while (!_window->should_close()) {
    InputHandler::instance().poll();
    ResLoader::instance().do_requests();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    double curr_frame = _window->get_time();
    double dt = curr_frame - _last_frame;
    _last_frame = curr_frame;

    glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _scene->update(dt);

    if (_window->imgui_demo) {
      ImGui::ShowDemoWindow(&_window->imgui_demo);
    }
    _scene->ui_draw();

    ImGui::Render();

    _window->swap_buffers();
  }
  Log::info("[Shogle] Terminating program");
}

} // namespace ntf