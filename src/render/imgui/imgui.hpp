#pragma once

#include "../../core.hpp"

#include <imgui.h>

namespace ntf {

template<typename ImguiImpl, typename... Args>
bool imgui_init(Args&&... args) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  ImGui::StyleColorsDark();
  return ImguiImpl::init(std::forward<Args>(args)...);
}

template<typename ImguiImpl>
void imgui_destroy() {
  ImguiImpl::destroy();
  ImGui::DestroyContext();
}

template<typename ImguiImpl>
void imgui_start_frame(ImguiImpl&& impl) {
  impl.start_frame();
  ImGui::NewFrame();
}

template<typename ImguiImpl>
void imgui_end_frame(ImguiImpl&& impl) {
  ImGui::Render();
  impl.end_frame();
}

} // namespace ntf
