#pragma once

#include <shogle/core/types.hpp>

#include <glad/glad.h>

namespace ntf::shogle {

template<typename T, typename... TRes>
concept same_as_any = (... or std::same_as<T, TRes>);

template<typename T>
concept vertex_type = (same_as_any<T, vec2, vec3, vec4>);

template<unsigned int _index, typename T>
requires(vertex_type<T>)
struct shader_attribute {
  static constexpr unsigned int index = _index;
  static constexpr size_t stride = sizeof(T);
};

template<typename... T>
struct stride_sum { static constexpr size_t value = 0; };

template<typename T, typename... U >
struct stride_sum<T, U...> { static constexpr size_t value = T::stride + stride_sum<U...>::value; };

template<class T>
concept is_shader_attribute = requires(T x) { 
  { shader_attribute{x} } -> std::same_as<T>;
};

enum class clear : uint8_t {
  none = 0,
  depth = 1 << 0,
  stencil = 1 << 1,
};

enum class depth_fun {
  less = 0,
  lequal,
};

constexpr clear operator|(clear a, clear b) {
  return static_cast<clear>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

constexpr clear operator&(clear a, clear b) {
  return static_cast<clear>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

constexpr clear& operator|=(clear& a, clear b) {
  return a = static_cast<clear>(a | b);
}

constexpr GLint __enumtogl(depth_fun depth) {
  switch (depth) {
    case depth_fun::less:
      return GL_LESS;
    case depth_fun::lequal:
      return GL_LEQUAL;
  }
}

inline void render_viewport(vec2sz sz) {
  glViewport(0, 0, sz.w, sz.h);
}

inline void render_viewport(size_t w, size_t h) {
  glViewport(0, 0, w, h);
}

inline void render_clear(color4 color, clear flag = clear::none) {
  GLbitfield mask = GL_COLOR_BUFFER_BIT;
  if ((flag & clear::depth) != clear::none) {
    mask |= GL_DEPTH_BUFFER_BIT;
  }
  if ((flag & clear::stencil) != clear::none) {
    mask |= GL_STENCIL_BUFFER_BIT;
  }
  glClearColor(color.r, color.g, color.b, color.a);
  glClear(mask);
}

inline void render_clear(color3 color, clear flag = clear::none) {
  render_clear(color4{color, 1.0f}, flag);
}

inline void render_stencil_test(bool flag) {
  if (flag) {
    glEnable(GL_STENCIL_TEST);
  } else {
    glDisable(GL_STENCIL_TEST);
  }
}

inline void render_depth_test(bool flag) {
  if (flag) {
    glEnable(GL_DEPTH_TEST);
  } else {
    glDisable(GL_DEPTH_TEST);
  }
}

inline void render_blending(bool flag) {
  if (flag) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //TODO: render_blend_fun
  } else {
    glDisable(GL_BLEND);
  }
}

inline void render_depth_fun(depth_fun fun) {
  switch (fun) {
    case depth_fun::less: {
      glDepthFunc(GL_LESS);
      break;
    }
    case depth_fun::lequal: {
      glDepthFunc(GL_LEQUAL);
      break;
    }
  }
}

} // namespace ntf::shogle
