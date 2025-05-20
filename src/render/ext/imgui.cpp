#include "../internal/platform.hpp"
#include "../../../include/shogle/boilerplate.hpp"
#include "./imgui.hpp"

#include <imgui_impl_opengl3.h> // Should work fine with OGL 4.x

// TODO: Define inits for vulkan & software renderer
#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
#include <imgui_impl_glfw.h>
#define SHOGLE_INIT_IMGUI_OPENGL(win, cbks) \
  ImGui_ImplGlfw_InitForOpenGL(reinterpret_cast<GLFWwindow*>(win), cbks); \
  ImGui_ImplOpenGL3_Init("#version 150")
#define SHOGLE_INIT_IMGUI_VULKAN(win, cbks)
#define SHOGLE_INIT_IMGUI_OTHER(win, cbks)

#define SHOGLE_DESTROY_IMGUI_OPENGL() \
  ImGui_ImplOpenGL3_Shutdown(); \
  ImGui_ImplGlfw_Shutdown()
#define SHOGLE_DESTROY_IMGUI_VULKAN() \
  NTF_UNREACHABLE()
#define SHOGLE_DESTROY_IMGUI_OTHER() \
  NTF_UNREACHABLE()

#define SHOGLE_IMGUI_OPENGL_NEW_FRAME() \
  ImGui_ImplGlfw_NewFrame(); \
  ImGui_ImplOpenGL3_NewFrame()
#define SHOGLE_IMGUI_VULKAN_NEW_FRAME() \
  NTF_UNREACHABLE()
#define SHOGLE_IMGUI_OTHER_NEW_FRAME() \
  NTF_UNREACHABLE()

#define SHOGLE_IMGUI_OPENGL_END_FRAME(draw_data) \
  ImGui_ImplOpenGL3_RenderDrawData(draw_data)
#define SHOGLE_IMGUI_VULKAN_END_FRAME(draw_data) \
  NTF_UNREACHABLE()
#define SHOGLE_IMGUI_OTHER_END_FRAME(draw_data) \
  NTF_UNREACHABLE()
#endif

namespace ntf {

imgui_ctx::imgui_ctx(r_context ctx, r_api shogle_api) noexcept :
  _ctx{ctx}, _shogle_api{shogle_api}, _draw_data{nullptr} {}

imgui_ctx::~imgui_ctx() noexcept { _destroy(); }

imgui_ctx::imgui_ctx(imgui_ctx&& other) noexcept :
  _ctx{std::move(other._ctx)}, _shogle_api{std::move(other._shogle_api)},
  _draw_data{std::move(other._draw_data)} { other._ctx = nullptr; }

imgui_ctx& imgui_ctx::operator=(imgui_ctx&& other) noexcept {
  _destroy();

  _ctx = std::move(other._ctx);
  _shogle_api = std::move(other._shogle_api);
  _draw_data = std::move(other._draw_data);
  
  other._ctx = nullptr;

  return *this;
}

imgui_ctx imgui_ctx::create(r_context ctx, ImGuiConfigFlags flags, bool bind_callbacks) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= flags;
  ImGui::StyleColorsDark();

  const auto win = r_get_window(ctx);
  const auto api = r_get_api(ctx);
  NTF_ASSERT(win);
  switch (api) {
    case r_api::opengl: {
      SHOGLE_INIT_IMGUI_OPENGL(win, bind_callbacks);
      break;
    }
    case r_api::vulkan: {
      SHOGLE_INIT_IMGUI_VULKAN(win, bind_callbacks);
      break;
    }
    default: {
      SHOGLE_INIT_IMGUI_OTHER(win, bind_callbacks);
      break;
    }
  }
  return imgui_ctx{ctx, api};
}

void imgui_ctx::_destroy() noexcept {
  if (!_ctx) {
    return;
  }

  switch (_shogle_api) {
    case r_api::opengl: {
      SHOGLE_DESTROY_IMGUI_OPENGL();
      break;
    }
    case r_api::vulkan: {
      SHOGLE_DESTROY_IMGUI_VULKAN();
      break;
    }
    default: {
      SHOGLE_DESTROY_IMGUI_OTHER();
      break;
    }
  }
  ImGui::DestroyContext();
}

void imgui_ctx::start_frame() {
  _draw_data = nullptr;
  switch (_shogle_api) {
    case r_api::opengl: {
      SHOGLE_IMGUI_OPENGL_NEW_FRAME();
      break;
    }
    case r_api::vulkan: {
      SHOGLE_IMGUI_VULKAN_NEW_FRAME();
      break;
    }
    default: {
      SHOGLE_IMGUI_OTHER_NEW_FRAME();
      break;
    }
  }
  ImGui::NewFrame();
}

void imgui_ctx::operator()(r_context, r_platform_handle) {
  NTF_ASSERT(_draw_data);
  switch (_shogle_api) {
    case r_api::opengl: {
      SHOGLE_IMGUI_OPENGL_END_FRAME(_draw_data);
      break;
    }
    case r_api::vulkan: {
      SHOGLE_IMGUI_VULKAN_END_FRAME(_draw_data);
      break;
    }
    default: {
      SHOGLE_IMGUI_OTHER_END_FRAME(_draw_data);
    }
  }
}

void imgui_ctx::end_frame(r_framebuffer target, const r_external_state* state,
                          ImDrawData* draw_data) {
  _draw_data = draw_data;
  if (!_draw_data) {
    ImGui::Render();
    _draw_data = ImGui::GetDrawData();
  }
  target = target ? target : r_get_default_framebuffer(_ctx);

  // hack
  const auto fb = r_framebuffer_get_viewport(target);
  _draw_data->DisplaySize.x = fb.z;
  _draw_data->DisplaySize.y = fb.w;

  r_submit_external_command(_ctx, {
    .target = target,
    .state = state ? *state : r_def_ext_state,
    .callback = {*this},
  });
}

} // namespace ntf
