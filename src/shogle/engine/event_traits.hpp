#pragma once

#include <shogle/render/glfw/window.hpp>

#include <concepts>

namespace ntf::shogle {

template<typename F>
concept renderfunc = std::invocable<F, shogle::window&, double, double>;

template<typename F>
concept updatefunc = std::invocable<F, shogle::window&, double>;

template<typename F>
concept viewportfunc = std::invocable<F, size_t, size_t>;

template<typename F>
concept keyfunc = std::invocable<F, keycode, scancode, keystate, keymod>;

template<typename F>
concept cursorfunc = std::invocable<F, double, double>;

template<typename F>
concept scrollfunc = std::invocable<F, double, double>;

} // namespace ntf::shogle
