#include "./opengl.hpp"

#if SHOGLE_USE_GLFW
#include <imgui_impl_glfw.h>
#endif

#include <imgui_impl_opengl3.h> // Should work fine with OGL 4.x

namespace ntf {

void r_context::init(r_window& win, const r_context_params& params) noexcept {
  r_api api = params.use_api.value_or(r_api::opengl);
  r_window::ctx_params_t win_params {
    .api = api,
    .gl_maj = 4,
    .gl_min = 6,
  };
  if (!win.init_context(win_params)) {
    SHOGLE_LOG(error, "[ntf::r_context] Failed to initialize window context");
    return;
  }

  try {
    switch (api) {
      case r_api::opengl: {
        _ctx = std::make_unique<gl_context>(win, 4, 6);
        break;
      }
      default: {
        NTF_ASSERT(false, "Not implemented");
        break;
      }
    }
  } catch (const std::exception& err) {
    win.reset();
    SHOGLE_LOG(error, "[ntf::r_context] Failed to create context: {}", err.what());
    return;
  }

#if SHOGLE_ENABLE_IMGUI
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();

#if SHOGLE_USE_GLFW
  switch (api) {
    case r_api::opengl: {
      ImGui_ImplGlfw_InitForOpenGL(win._handle, true);
      ImGui_ImplOpenGL3_Init("#version 130");
      break;
    }
    case r_api::vulkan: {
      ImGui_ImplGlfw_InitForVulkan(win._handle, true);
      break;
    }
    default: {
      ImGui_ImplGlfw_InitForOther(win._handle, true);
      break;
    }
  }
#endif
#endif
  _win = &win;
  _ctx_api = api;
}

r_context::~r_context() noexcept {
  if (!_ctx) {
    return;
  }
  _ctx.reset();

#if SHOGLE_ENABLE_IMGUI
  switch (_ctx_api) {
    case r_api::opengl: {
      ImGui_ImplOpenGL3_Shutdown();
      break;
    }
    default: break;
  }
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
#endif

  _win->reset();
}

void r_context::start_frame() noexcept {
  _uniforms.clear();
  _fb_cmds.clear();
  _frame_arena.reset();

#if SHOGLE_ENABLE_IMGUI
#if SHOGLE_USE_GLFW
  ImGui_ImplGlfw_NewFrame();
#endif
  switch (_ctx_api) {
    case r_api::opengl: ImGui_ImplOpenGL3_NewFrame(); break;
    // case r_api::vulkan: ImGui_ImplVulkan_NewFrame(); break;
    default: break;
  }
  ImGui::NewFrame();
#endif
}

void r_context::device_wait() noexcept {
  _ctx->device_wait();
}

} // namespace ntf
