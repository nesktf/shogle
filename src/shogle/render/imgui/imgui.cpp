#include <shogle/render/imgui/imgui.hpp>

namespace ntf::shogle::imgui {

renderer::renderer(const glfw::window& win) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(win._handle, true);
  ImGui_ImplOpenGL3_Init("#version 130");
}

renderer::~renderer() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

} // namespace ntf::shogle
