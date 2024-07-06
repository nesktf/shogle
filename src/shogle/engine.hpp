#pragma once

#include <shogle/render/render.hpp>

#include <shogle/keys.hpp>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <functional>

namespace ntf::shogle {

void engine_init(size_t width, size_t height, std::string_view title);
void engine_destroy();

void engine_viewport_event(std::function<void(size_t,size_t)> fun);
void engine_key_event(std::function<void(keycode,scancode,keystate,keymod)> fun);
void engine_cursor_event(std::function<void(double,double)> fun);
void engine_scroll_event(std::function<void(double,double)> fun);

bool engine_poll_key(keycode key);
vec2sz engine_window_size();
void engine_use_vsync(bool flag);
void engine_set_title(std::string_view title);
void engine_close_window();

GLFWwindow* __engine_window_handle();

template<typename F>
concept fixrenderfunc = std::invocable<F, double, double, double>;

template<typename F>
concept renderfunc = std::invocable<F, double>;

template<typename F>
concept updatefunc = std::invocable<F, double>;

struct __deffun {
  template<typename... Args>
  void operator()(Args... args) {}
};

template<fixrenderfunc RFunc, updatefunc FUFunc, updatefunc UFunc = __deffun>
void engine_main_loop(uint ups, RFunc&& render, FUFunc&& fixed_update, UFunc&& update = {});

template<renderfunc RFunc, updatefunc UFunc>
void engine_main_loop(RFunc&& render, UFunc&& update);

} // namespace ntf::shogle

#ifndef SHOGLE_ENGINE_INL_HPP
#include <shogle/engine.inl.hpp>
#endif
