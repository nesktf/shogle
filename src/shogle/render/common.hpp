#pragma once

#include <shogle/shogle.hpp>
#include <shogle/math/alg.hpp>

#define SHOGLE_CUBEMAP_FACES 6u

namespace ntf {

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

enum class depth_fun {
  less = 0,
  lequal,
};

enum class tex_format {
  mono = 0,
  rgb,
  rgba,
};

enum class tex_filter {
  nearest = 0,
  linear,
  nearest_mp_nearest,
  nearest_mp_linear,
  linear_mp_linear,
  linear_mp_nearest,
};

enum class tex_wrap {
  repeat = 0,
  mirrored_repeat,
  clamp_edge,
  clamp_border,
};

enum class shader_category {
  vertex = 0,
  fragment,
  geometry
};

enum class uniform_category {
  scalar = 0, // float
  iscalar, // int
  vec2,
  vec3, 
  vec4,
  mat3,
  mat4,
};

enum class mesh_buffer {
  static_draw = 0,
  dynamic_draw,
  stream_draw,
};

enum class mesh_primitive {
  triangles = 0,
  triangle_strip,
  triangle_fan,
  lines,
  line_strip,
  line_loop,
  points,
};

enum class material_category {
  diffuse = 0,
  specular
};

struct font_glyph {
  ivec2 size;
  ivec2 bearing;
  unsigned long advance;
};


template<typename T>
struct uniform_traits {
  static constexpr bool is_uniform = false;
};

template<>
struct uniform_traits<float> {
  static constexpr bool is_uniform = true;
  static constexpr uniform_category enum_value = uniform_category::scalar;
  static constexpr size_t size = sizeof(float);
};

template<>
struct uniform_traits<int> {
  static constexpr bool is_uniform = true;
  static constexpr uniform_category enum_value = uniform_category::iscalar;
  static constexpr size_t size = sizeof(int);
};

template<>
struct uniform_traits<vec2> {
  static constexpr bool is_uniform = true;
  static constexpr uniform_category enum_value = uniform_category::vec2;
  static constexpr size_t size = sizeof(vec2);
};

template<>
struct uniform_traits<vec3> {
  static constexpr bool is_uniform = true;
  static constexpr uniform_category enum_value = uniform_category::vec3;
  static constexpr size_t size = sizeof(vec3);
};

template<>
struct uniform_traits<vec4> {
  static constexpr bool is_uniform = true;
  static constexpr uniform_category enum_value = uniform_category::vec4;
  static constexpr size_t size = sizeof(vec4);
};

template<>
struct uniform_traits<mat3> {
  static constexpr bool is_uniform = true;
  static constexpr uniform_category enum_value = uniform_category::mat3;
  static constexpr size_t size = sizeof(mat3);
};

template<>
struct uniform_traits<mat4> {
  static constexpr bool is_uniform = true;
  static constexpr uniform_category enum_value = uniform_category::mat4;
  static constexpr size_t size = sizeof(mat4);
};


template<unsigned int Index, vertex_type T>
struct shader_attribute {
  static constexpr unsigned int index = Index;
  static constexpr size_t stride = sizeof(T);
};

template<class T>
concept is_shader_attribute = requires(T x) { 
  { shader_attribute{x} } -> std::same_as<T>;
};

template<typename... T>
struct stride_sum {
  static constexpr size_t value = 0;
};

template<typename T, typename... U >
struct stride_sum<T, U...> {
  static constexpr size_t value = T::stride + stride_sum<U...>::value;
};


template<typename Shader>
class uniform_tuple {
public:
  using shader_type = Shader;
  using uniform_type = typename Shader::uniform_type;

  using entry = std::pair<uniform_type, uniform_category>;

public:
  uniform_tuple() = default;

  uniform_tuple(std::vector<entry> list);

public:
  void bind(const shader_type& shader) const;
  void clear();

  template<typename T>
  requires(uniform_traits<T>::is_uniform)
  bool set_uniform(uniform_type uniform, T&& val);

public:
  size_t size() const { return _uniforms.size(); }
  bool empty() const { return size() == 0; }

  operator bool() const { return !empty(); }

private:
  std::unordered_map<uniform_type, std::pair<uniform_category, size_t>> _uniforms;
  uint8_t* _data;

public:
  NTF_DECLARE_MOVE_ONLY(uniform_tuple);
};


static constexpr float cube_vertices[] {
  // coord               // normal             // texcoord
  -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
   0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
   0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
   0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
  -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,
  -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,

  -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
   0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
   0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
   0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
  -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
  -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,

  -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
  -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
  -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
  -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
  -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
  -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

   0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
   0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
   0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
   0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
   0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
   0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

  -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
   0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
   0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
   0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
  -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
  -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,

  -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
   0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
   0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
   0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
  -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f,
  -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f
};

static constexpr float quad_vertices[] {
  // coord              //normals           // texcoord   // inv texcoord
  -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   /* 0.0f, 1.0f, // + vec2{0.0f, 1.0f} */
   0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,   /* 1.0f, 1.0f, // + vec2{0.0f, 1.0f} */
   0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,   /* 1.0f, 0.0f, // + vec2{0.0f, -1.0f} */
  -0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,   /* 0.0f, 0.0f, // + vec2{0.0f, -1.0f} */
};

static constexpr uint quad_indices[] {
  0, 1, 2, // bottom right triangle
  0, 2, 3  // top left triangle
};

} // namespace ntf

#ifndef SHOGLE_RENDER_COMMON_INL
#include <shogle/render/common.inl>
#endif
