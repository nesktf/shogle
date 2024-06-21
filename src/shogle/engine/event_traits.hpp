#pragma once

#include <shogle/render/glfw/keys.hpp>

#include <concepts>

namespace ntf::shogle {

template<typename F>
concept updatefun = std::invocable<F, float>;

template<typename F>
concept drawfun = std::invocable<F>;

template<typename F>
concept viewportfun = std::invocable<F, size_t, size_t>;

template<typename F>
concept keyfun = std::invocable<F, keycode, scancode, keystate, keymod>;

template<typename F>
concept cursorfun = std::invocable<F, double, double>;

template<typename F>
concept scrollfun = std::invocable<F, double, double>;

}
