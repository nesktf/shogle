#pragma once

#include <shogle/render/common.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace shogle {

constexpr inline ImGuiConfigFlags DEFAULT_IMGUI_CONFIG =
  ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;

template<render_context_tag tag>
void imgui_init(GLFWwindow* win, ImGuiConfigFlags flags = DEFAULT_IMGUI_CONFIG,
                bool bind_callbacks = true) {
  NTF_ASSERT(win);
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags = flags;
  ImGui::StyleColorsDark();
  if constexpr (tag == render_context_tag::opengl) {
    ImGui_ImplGlfw_InitForOpenGL(win, bind_callbacks);
    ImGui_ImplOpenGL3_Init("#version 150");
  } else {
    NTF_ASSERT(false, "TODO");
  }
}

template<render_context_tag tag>
void imgui_new_frame() {
  if constexpr (tag == render_context_tag::opengl) {
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
  } else {
    NTF_ASSERT(false, "TODO");
  }
  ImGui::NewFrame();
}

template<render_context_tag tag>
void imgui_end_frame() {
  if constexpr (tag == render_context_tag::opengl) {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  } else {
    NTF_ASSERT(false, "TODO");
  }
}

template<render_context_tag tag>
void imgui_destroy() {
  if constexpr (tag == render_context_tag::opengl) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
  } else {
    NTF_ASSERT(false, "TODO");
  }
  ImGui::DestroyContext();
}

} // namespace shogle
