#pragma once

#include <shogle/core/singleton.hpp>
#include <shogle/render/glfw.hpp>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

namespace ntf::render {

class imgui : public Singleton<imgui> {
public:
  imgui() = default;

public:
  static void init(const glfw::window& win);
  static void destroy(void);

public:
  static inline void new_frame(void) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }

  static inline void render(void) {
    if (imgui::instance().show_demo) {
      ImGui::ShowDemoWindow(&imgui::instance().show_demo);
    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }

public:
  static bool show_demo;
};

} // namespace ntf::render
