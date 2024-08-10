#pragma once

#include <shogle/core/types.hpp>

#include <glad/glad.h>

namespace ntf {

namespace impl {

bool render_init(GLADloadproc proc);
void render_destroy();

} // namespace impl

void render_viewport(size_t w, size_t h);
void render_viewport(size_t w0, size_t h0, size_t w, size_t h);

void render_stencil_test(bool flag);
void render_depth_test(bool flag);
void render_blending(bool flag);


enum class clear : uint8_t {
  none = 0,
  depth = 1 << 0,
  stencil = 1 << 1,
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

void render_clear(color4 color, clear flag = clear::none);
void render_clear(color3 color, clear flag = clear::none);


enum class depth_fun {
  less = 0,
  lequal,
};

void render_depth_fun(depth_fun fun);

void render_draw_quad();
void render_draw_cube();

template<typename T>
concept vertex_type = (ntf::same_as_any<T, vec2, vec3, vec4>);

template<unsigned int _index, typename T>
requires(vertex_type<T>)
struct shader_attribute {
  static constexpr unsigned int index = _index;
  static constexpr size_t stride = sizeof(T);
};

template<class T>
concept is_shader_attribute = requires(T x) { 
  { shader_attribute{x} } -> std::same_as<T>;
};

template<typename... T>
struct stride_sum { static constexpr size_t value = 0; };

template<typename T, typename... U >
struct stride_sum<T, U...> { static constexpr size_t value = T::stride + stride_sum<U...>::value; };


using shadatt_coords3d    = shader_attribute<0, vec3>;
using shadatt_normals3d   = shader_attribute<1, vec3>;
using shadatt_texcoords3d = shader_attribute<2, vec2>;

} // namespace ntf
