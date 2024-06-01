#pragma once

#include <shogle/render/glfw/window.hpp>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

namespace ntf::shogle::imgui {

class renderer {
public:
  renderer(const glfw::window& win);
  ~renderer();

public:
  inline void render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }

  inline void new_frame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }
};

} // namespace ntf::shogle::imgui
