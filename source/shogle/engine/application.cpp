#include <shogle/render/gl/render.hpp>
#include <shogle/render/api/imgui.hpp>
#include <shogle/engine/application.hpp>

namespace ntf::shogle {

application::application(size_t w, size_t h, std::string title) :
  _window(glfw::create_window(vec2sz{w, h}, std::move(title))) {}

application::~application() {
  glfw::destroy_window(_window);
}

void application::terminate() {
  glfw::close_window(_window);
}

void application::main_loop() {
  imgui::init(_window);

  glfw::set_user_ptr(_window, this);

  glfw::set_key_callback(_window, [](auto* win, int key, int, int action, int) {
    auto* _this = static_cast<application*>(glfw::get_user_ptr(win));
    _this->input_event(key, action);
  });

  glfw::set_viewport_callback(_window, [](auto* win, int w, int h) {
    auto* _this = static_cast<application*>(glfw::get_user_ptr(win));
    gl::set_viewport_size(vec2sz{w, h});
    _this->viewport_event((size_t)w, (size_t)h);
  });

  glfw::set_cursor_callback(_window, [](auto* win, double xpos, double ypos) {
    auto* _this = static_cast<application*>(glfw::get_user_ptr(win));
    _this->cursor_event(xpos, ypos);
  });

  glfw::set_scroll_callback(_window, [](auto* win, double xoff, double yoff) {
    auto * _this = static_cast<application*>(glfw::get_user_ptr(win));
    _this->scroll_event(xoff, yoff);
  });

  while (glfw::is_window_open(_window)) {
    glfw::poll_events(_window);
    imgui::new_frame();

    double _curr_frame = glfw::elapsed_time(_window);
    double dt = _curr_frame - _last_frame;
    _last_frame = _curr_frame;

    draw_event();
    update_event(dt);

    imgui::render();
    glfw::swap_buffers(_window);
  }

  imgui::terminate();
}

} // namespace ntf::shogle
