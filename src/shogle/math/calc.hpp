#pragma once

#include "./alg.hpp"

namespace ntf {

// TODO: integrator traits?
template<typename T, typename Fun>
concept integr_fun = std::is_invocable_r_v<T, Fun, T>;
// f(T x) -> T

template<typename T>
struct integr_trap {
  template<integr_fun<T> Fun>
  constexpr T operator()(T a, T b, Fun&& f) {
    const T size = b-a;
    return (size/2)*(f(a)+f(b));
  }

  template<integr_fun<T> Fun>
  constexpr T operator()(T a, T b, uint n, Fun&& f) {
    const T size = b-a;
    const T h = size/static_cast<T>(n);

    T out = f(a);
    for (uint i = 1; i < n; ++i) {
      out += 2*f(a+(i*h));
    }
    out += f(b);

    return (size*out)/(2*n);
  }
};

template<typename T>
struct integr_simp13 {
  template<integr_fun<T> Fun>
  constexpr T operator()(T a, T b, Fun&& f) {
    const T size = b-a;
    const T x_1 = a+((b-a)/3);

    return (size/6)*(f(a)+4*f(x_1)+f(b));
  }

  template<integr_fun<T> Fun>
  constexpr T operator()(T a, T b, uint n, Fun&& f) {
    const T size = b-a;
    const T h = size/static_cast<T>(n);

    T out = f(a);
    for (uint i = 1; i < n; i+=2) {
      out += 4*f(a+(i*h)); // Odd
    }
    for (uint i = 2; i < n; i+=2) {
      out += 2*f(a+(i*h)); // Even
    }
    out += f(b);

    return (size*out)/(3*n);
  }
};

template<typename T>
struct integr_simp38 {
  template<integr_fun<T> Fun>
  constexpr T operator()(T a, T b, Fun&& f) {
    const T size = b-a;
    const T h = size/4;
    const T x_1 = a+h;
    const T x_2 = a+(h*2);

    return (size/8)*(f(a)+3*f(x_1)+3*f(x_2)+f(b));
  }
};


// TODO: ODE system solvers
template<typename T, typename Fun>
concept ode_fun = std::is_invocable_r_v<T, Fun, T, T>;
// f(T x, T y) -> T
// \phi(x, y, h) = f(x, y)

template<typename T>
struct ode_rk4 {
  template<ode_fun<T> Fun>
  constexpr T operator()(T x, T y, T h, Fun&& f) {
    const auto k1 = h*f(x, y);
    const auto k2 = h*f(x + h*T{0.5}, y + k1*T{0.5});
    const auto k3 = h*f(x + h*T{0.5}, y + k2*T{0.5});
    const auto k4 = h*f(x + h, y + k3);

    return y + T{1/6.0}*(k1 + 2*k2 + 2*k3 + k4);
  }
};

template<typename T>
struct ode_euler {
  template<ode_fun<T> Fun>
  constexpr T operator()(T x, T y, T h, Fun&& f) {
    return y + h*f(x, y);
  }
};

} // namespace ntf
