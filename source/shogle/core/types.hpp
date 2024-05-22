#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

#include <memory>
#include <complex>

#include <sys/types.h>

#define PI M_PIf
#define I cmplx{0.0f, 1.0f}

namespace ntf::shogle {

// Might replace glm at some point (?)
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using mat3 = glm::mat3;

using cmplx = std::complex<float>;
using quat = glm::quat;

using color = vec4;
using color4 = vec4;
using color3 = vec3;

using path_t = std::string;

struct vec2i {
  int x{};
  int y{};
};

struct vec2sz {
  vec2sz(int w, int h) :
    x((size_t)w), y((size_t)h) {}
  vec2sz(size_t w, size_t h) :
    x(w), y(h) {}
  union {
    size_t x;
    size_t w;
  };
  union {
    size_t y;
    size_t h;
  };
  operator vec2() { return vec2{(float)w, (float)h}; }
  operator cmplx() { return cmplx{(float)w, (float) h}; }
};

template<typename T>
using wptr = T*; // non owning pointer

template<typename T>
using uptr = std::unique_ptr<T>;

template<typename T>
using sptr = std::shared_ptr<T>;

template<typename T, typename... Args>
inline T* make_ptr(Args&&... args) {
  return new T{std::forward<Args>(args)...};
}

template<typename T, typename... Args>
inline uptr<T> make_uptr(Args&&... args) {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

} // namespace ntf::shogle
