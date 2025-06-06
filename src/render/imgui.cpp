#include "./internal/platform.hpp"
#include "../../../include/shogle/boilerplate.hpp"
#include "./imgui.hpp"

#include <imgui_impl_opengl3.h> // Should work fine with OGL 4.x

// TODO: Define inits for vulkan & software renderer
#if defined(SHOGLE_ENABLE_GLFW) && SHOGLE_ENABLE_GLFW
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

namespace ntf::render {

imgui_ctx::imgui_ctx(context_view ctx, context_api shogle_api) noexcept :
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

imgui_ctx imgui_ctx::create(context_view ctx, ImGuiConfigFlags flags, bool bind_callbacks) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= flags;
  ImGui::StyleColorsDark();

  const auto win = ctx.window();
  const auto api = ctx.api();
  NTF_ASSERT(win);
  switch (api) {
    case context_api::opengl: {
      SHOGLE_INIT_IMGUI_OPENGL(win, bind_callbacks);
      break;
    }
    case context_api::vulkan: {
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
    case context_api::opengl: {
      SHOGLE_DESTROY_IMGUI_OPENGL();
      break;
    }
    case context_api::vulkan: {
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
    case context_api::opengl: {
      SHOGLE_IMGUI_OPENGL_NEW_FRAME();
      break;
    }
    case context_api::vulkan: {
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

void imgui_ctx::operator()(context_t, ctx_handle) {
  NTF_ASSERT(_draw_data);
  switch (_shogle_api) {
    case context_api::opengl: {
      SHOGLE_IMGUI_OPENGL_END_FRAME(_draw_data);
      break;
    }
    case context_api::vulkan: {
      SHOGLE_IMGUI_VULKAN_END_FRAME(_draw_data);
      break;
    }
    default: {
      SHOGLE_IMGUI_OTHER_END_FRAME(_draw_data);
    }
  }
}

void imgui_ctx::end_frame(framebuffer_view target,
                          uint32 sort_group,
                          weak_cptr<external_state> state,
                          ImDrawData* draw_data) {
  _draw_data = draw_data;
  if (!_draw_data) {
    ImGui::Render();
    _draw_data = ImGui::GetDrawData();
  }
  if (target.empty()) {
    target = framebuffer::get_default(_ctx);
  }

  // hack
  const auto fb = target.viewport();
  _draw_data->DisplaySize.x = fb.z;
  _draw_data->DisplaySize.y = fb.w;

  _ctx.submit_external_command({
    .target = target,
    .state = state,
    .sort_group = sort_group,
    .render_callback = {*this},
  });
}

} // namespace ntf::render
