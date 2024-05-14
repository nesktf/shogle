#pragma once

#include <shogle/core/singleton.hpp>
#include <shogle/render/glfw.hpp>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

namespace ntf::shogle::imgui {

void init(const glfw::window& win);
void terminate();

inline void new_frame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

inline void render() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

} // namespace ntf::shogle::imgui
