#pragma once

#include <shogle/core/singleton.hpp>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

namespace ntf::render {

class imgui : public Singleton<imgui> {
public:
  imgui() = default;

public:
  void init(GLFWwindow* win);
  void destroy(void);

public:
  static inline void new_frame(void) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }

  static inline void render(void) {
    if (imgui::instance().toggle_demo) {
      ImGui::ShowDemoWindow(&imgui::instance().toggle_demo);
    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }

public:
  bool toggle_demo {true};
};


} // namespace ntf::render
