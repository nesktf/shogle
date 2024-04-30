#pragma once

#include <shogle/core/singleton.hpp>

#include <shogle/render/backends/imgui.hpp>

#include <GLFW/glfw3.h>

namespace ntf::input {
  class glfw;
}

namespace ntf::render {

class glfw : public Singleton<glfw> {
public:
  using vportcallback_t = GLFWframebuffersizefun;

public:
  void init(size_t width, size_t height);
  void destroy();

  void set_viewport_callback(vportcallback_t cb);
  void set_title(const char* title);

public:
  inline void new_frame(void) {
    glfwPollEvents();
    imgui::new_frame();
  }

  inline void end_frame() {
    imgui::render();
    glfwSwapBuffers(_win);
  }

  inline bool is_active(void) {
    return glfwWindowShouldClose(_win);
  }
  
  inline double elapsed_time(void) {
    return glfwGetTime();
  }

private:
  GLFWwindow* _win;
  size_t _width, _height;
  friend class input::glfw;
};

} // namespace ntf::render
