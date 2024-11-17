#pragma once

#include <shogle/core/common.hpp>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

namespace ntf {

class imgui {
public:
  template<typename Impl>
  class imgui_lib;

  struct glfw_gl3_impl;
  
public:
  template<typename Window, typename Impl>
  [[nodiscard]] static imgui_lib<Impl> init(Window& win, Impl impl);
};

struct imgui::glfw_gl3_impl {
  static void init(GLFWwindow* win, bool install_callbacks) {
    ImGui_ImplGlfw_InitForOpenGL(win, install_callbacks);
    ImGui_ImplOpenGL3_Init("#version 130");
  }

  static void destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
  }

  void start_frame() const {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
  }

  void end_frame() const {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }
};

template<typename Impl>
class imgui::imgui_lib {
private:
  imgui_lib() = default;

public:
  void start_frame() const { 
    impl.start_frame();
    ImGui::NewFrame();
  }
  void end_frame() const {
    ImGui::Render();
    impl.end_frame();
  }

private:
  Impl impl{};

private:
  friend class imgui;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(imgui_lib);
};

template<typename Window, typename Impl>
[[nodiscard]] auto imgui::init(Window& win, Impl impl) -> imgui_lib<Impl> {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void) io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  ImGui::StyleColorsDark();

  win.imgui_init(impl);
  return imgui_lib<Impl>{};
}

template<typename Impl>
imgui::imgui_lib<Impl>::~imgui_lib() noexcept {
  Impl::destroy();
  ImGui::DestroyContext();
}

} // namespace ntf
