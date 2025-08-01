#pragma once

#include <shogle/math/vector.hpp>

namespace shogle {

constexpr inline f32 rad(f32 deg) {
  return glm::radians(deg);
}

constexpr inline f32 deg(f32 rad) {
  return glm::degrees(rad);
}

template<typename TL, typename TR>
constexpr auto periodic_add(TL a, TR b, decltype(a+b) min, decltype(a+b) max) {
  auto res = a + b;
  auto range = max-min;
  while (res >= max) res -= range;
  while (res <  min) res += range;
  return res;
}

template<typename T>
constexpr bool equal(T f1, T f2) { 
  return (std::fabs(f1 - f2) <= std::numeric_limits<T>::epsilon() * std::fmax(std::fabs(f1),
                                                                              std::fabs(f2)));
}

template<typename T>
constexpr T epserr(const T& a, const T& b) {
  return std::abs((b-a)/b);
}


constexpr inline bool collision2d(vec2 pos1, vec2 size1, vec2 pos2, vec2 size2) {
  // AABB assumes pos is the lower left corner
  // Normalize scale, since obj.pos it's the quad's center
  vec2 normsize1 = size1 / 2.0f;
  vec2 normsize2 = size2 / 2.0f;

  bool col_x = 
    pos1.x + normsize1.x >= (pos2.x - normsize2.x) &&
    pos2.x + normsize2.x >= (pos1.x - normsize1.x);
  bool col_y = 
    pos1.y + normsize1.y >= (pos2.y - normsize2.y) &&
    pos2.y + normsize2.y >= (pos1.y - normsize1.y);

  return col_x && col_y;
}

constexpr inline bool collision2d(vec2 pos1, vec2 size1, vec2 pos2, float rad2) {
  // No need to normalize pos1 to be the center
  float sq_rad = rad2*rad2; // mult is cheaper than sqrt?
  vec2 half_rect = size1 / 2.0f;

  vec2 diff = pos2 - pos1;
  vec2 diff_clamp = glm::clamp(diff, -half_rect, half_rect);
  vec2 closest_p = pos1 + diff_clamp;

  diff = closest_p - pos2;
  float sq_len = (diff.x*diff.x) + (diff.y*diff.y);

  return sq_len < sq_rad;
}

constexpr inline bool collision2d(vec2 pos1, float rad1, vec2 pos2, vec2 size2) {
  return collision2d(pos2, size2, pos1, rad1);
}

constexpr inline bool collision2d(vec2 pos1, float rad1, vec2 pos2, float rad2) {
  vec2 diff = pos2 - pos1;

  float sq_sum = (rad1+rad2)*(rad1+rad2); // maybe it is
  float sq_len = (diff.x*diff.x) + (diff.y*diff.y);

  return sq_len < sq_sum;
}


// TODO: integrator traits?
template<typename Fun, typename T>
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
template<typename Fun, typename T>
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

} // namespace shogle
