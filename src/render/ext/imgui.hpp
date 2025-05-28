#pragma once

#include <imgui.h>

#include "../context.hpp"

namespace ntf {

class imgui_ctx {
private:
  imgui_ctx(r_context win, r_api shogle_api) noexcept;

public:
  imgui_ctx(imgui_ctx&& other) noexcept;
  imgui_ctx(const imgui_ctx&) = delete;

  ~imgui_ctx() noexcept;

public:
  static imgui_ctx create(r_context ctx,
                          ImGuiConfigFlags flags = ImGuiConfigFlags_NavEnableKeyboard,
                          bool bind_callbacks = true);

public:
  imgui_ctx& operator=(imgui_ctx&& other) noexcept;
  imgui_ctx& operator=(const imgui_ctx&) = delete;

public:
  void operator()(r_context, r_platform_handle);

public:
  void start_frame();
  void end_frame(r_framebuffer target = nullptr, 
                 uint32 sort_group = 0u,
                 weak_cref<r_external_state> state = nullptr,
                 ImDrawData* draw_data = nullptr);

private:
  void _destroy() noexcept;

private:
  r_context _ctx;
  r_api _shogle_api;
  ImDrawData* _draw_data;
};

} // namespace ntf
