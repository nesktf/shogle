#include <shogle/core/engine.hpp>
#include <shogle/core/log.hpp>
#include <shogle/core/error.hpp>

#include <shogle/input/input.hpp>

#include <shogle/scene/camera.hpp>

#include <shogle/res/loader.hpp>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

namespace ntf {

bool Shogle::init(const Settings& sett) {
  try {
    _window = make_uptr<render::window>(sett.w_width, sett.w_height, sett.w_title.c_str());
  } catch(const ntf::error& e) {  
    Log::error("{}", e.what());
    return false;
  }
  clear_color = sett.clear_color;

  Camera2D::default_cam.set_viewport({static_cast<float>(sett.w_width), static_cast<float>(sett.w_height)});
  Camera2D::default_cam.update({});

  Camera3D::default_cam.set_viewport({static_cast<float>(sett.w_width), static_cast<float>(sett.w_height)});
  Camera3D::default_cam.update({});

  Log::verbose("[Shogle] Settings applied");

  _window->set_fb_callback([](auto, int w, int h) {
    render::gl::set_viewport(w, h);

    Camera2D::default_cam.set_viewport({static_cast<float>(w), static_cast<float>(h)});
    Camera2D::default_cam.update({});

    Camera3D::default_cam.set_viewport({static_cast<float>(w), static_cast<float>(h)});
    Camera3D::default_cam.update({});

    Log::verbose("[Window] Viewport updated");
  });
  Log::verbose("[InputHandler] Framebuffer callback set");

  input::InputHandler::instance().init(_window.get());

  Log::info("[Shogle] Initialized");
  return true;
}

void Shogle::start(SceneCreator creator) {
  _scene = creator();
  Log::verbose("[Shogle] Initial level created");

  Log::info("[Shogle] Entering main loop");
  while (!_window->should_close()) {
    input::InputHandler::instance().poll();
    res::loader::instance().do_requests();


    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    double curr_frame = _window->get_time();
    double dt = curr_frame - _last_frame;
    _last_frame = curr_frame;

    render::gl::clear_viewport({clear_color, 1.0f});

    _scene->update(dt);

    if (_window->imgui_demo) {
      ImGui::ShowDemoWindow(&_window->imgui_demo);
    }
    _scene->draw_ui();

    ImGui::Render();

    _window->swap_buffers();
  }
  Log::info("[Shogle] Terminating program");
}

} // namespace ntf
