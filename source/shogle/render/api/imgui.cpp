#include <shogle/render/imgui.hpp>

namespace ntf::render {

void imgui::init(const glfw::window& win) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(win.glfw_win, true);
  ImGui_ImplOpenGL3_Init("#version 130");
}

void imgui::destroy(void) {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

} // namespace ntf::render
