#pragma once

#include <shogle/render/context.hpp>
#include <shogle/render/framebuffer.hpp>

#include <imgui.h>

namespace shogle {

class imgui_ctx {
private:
  imgui_ctx(context_view win, context_api shogle_api) noexcept;

public:
  imgui_ctx(imgui_ctx&& other) noexcept;
  imgui_ctx(const imgui_ctx&) = delete;

  ~imgui_ctx() noexcept;

public:
  static imgui_ctx create(context_view ctx, window_t win,
                          ImGuiConfigFlags flags = ImGuiConfigFlags_NavEnableKeyboard,
                          bool bind_callbacks = true);

public:
  imgui_ctx& operator=(imgui_ctx&& other) noexcept;
  imgui_ctx& operator=(const imgui_ctx&) = delete;

public:
  void operator()(context_t, ctx_handle);

public:
  void start_frame();
  void end_frame(framebuffer_view target = nullptr, 
                 uint32 sort_group = 0u,
                 weak_ptr<const external_state> state = nullptr,
                 ImDrawData* draw_data = nullptr);

private:
  void _destroy() noexcept;

private:
  context_view _ctx;
  context_api _shogle_api;
  ImDrawData* _draw_data;
};

} // namespace shogle
