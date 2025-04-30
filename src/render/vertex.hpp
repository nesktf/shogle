#pragma once

#include "./renderer.hpp"

#define SHOGLE_DECLARE_ATTRIB_TRAIT(_type, _tag) \
template<> \
struct r_attrib_traits<_type> { \
  static constexpr auto tag = _tag; \
  static constexpr size_t size = r_attrib_type_size(tag); \
  static constexpr uint32 dim = r_attrib_type_dim(tag); \
  static constexpr bool is_attrib = true; \
}

namespace ntf {

template<typename T>
struct r_attrib_traits {
  static constexpr bool is_attrib = false;
};

template<typename T>
concept shader_attribute_type = r_attrib_traits<T>::is_attrib;

constexpr inline size_t r_attrib_type_size(r_attrib_type type) {
  constexpr size_t int_sz = sizeof(int32);
  constexpr size_t float_sz = sizeof(float);
  constexpr size_t double_sz = sizeof(double);

  switch (type) {
    case r_attrib_type::i32:   return int_sz;
    case r_attrib_type::ivec2: return 2*int_sz;
    case r_attrib_type::ivec3: return 3*int_sz;
    case r_attrib_type::ivec4: return 4*int_sz;

    case r_attrib_type::f32:   return float_sz;
    case r_attrib_type::vec2:  return 2*float_sz;
    case r_attrib_type::vec3:  return 3*float_sz;
    case r_attrib_type::vec4:  return 4*float_sz;
    case r_attrib_type::mat3:  return 9*float_sz;
    case r_attrib_type::mat4:  return 16*float_sz;

    case r_attrib_type::f64:   return double_sz;
    case r_attrib_type::dvec2: return 2*double_sz;
    case r_attrib_type::dvec3: return 3*double_sz;
    case r_attrib_type::dvec4: return 4*double_sz;
    case r_attrib_type::dmat3: return 9*double_sz;
    case r_attrib_type::dmat4: return 16*double_sz;
  };

  return 0;
};

constexpr inline uint32 r_attrib_type_dim(r_attrib_type type) {
  switch (type) {
    case r_attrib_type::i32:   [[fallthrough]];
    case r_attrib_type::f32:   [[fallthrough]];
    case r_attrib_type::f64:   return 1;

    case r_attrib_type::vec2:  [[fallthrough]];
    case r_attrib_type::ivec2: [[fallthrough]];
    case r_attrib_type::dvec2: return 2;

    case r_attrib_type::vec3:  [[fallthrough]];
    case r_attrib_type::ivec3: [[fallthrough]];
    case r_attrib_type::dvec3: return 3;

    case r_attrib_type::vec4:  [[fallthrough]];
    case r_attrib_type::ivec4: [[fallthrough]];
    case r_attrib_type::dvec4: return 4;

    case r_attrib_type::mat3:  [[fallthrough]];
    case r_attrib_type::dmat3: return 9;

    case r_attrib_type::mat4:  [[fallthrough]];
    case r_attrib_type::dmat4: return 16;
  };

  return 0;
}

SHOGLE_DECLARE_ATTRIB_TRAIT(float32,  r_attrib_type::f32);
SHOGLE_DECLARE_ATTRIB_TRAIT(vec2,     r_attrib_type::vec2);
SHOGLE_DECLARE_ATTRIB_TRAIT(vec3,     r_attrib_type::vec3);
SHOGLE_DECLARE_ATTRIB_TRAIT(vec4,     r_attrib_type::vec4);
SHOGLE_DECLARE_ATTRIB_TRAIT(mat3,     r_attrib_type::mat3);
SHOGLE_DECLARE_ATTRIB_TRAIT(mat4,     r_attrib_type::mat4);

SHOGLE_DECLARE_ATTRIB_TRAIT(float64,  r_attrib_type::f64);
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec2,    r_attrib_type::dvec2);
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec3,    r_attrib_type::dvec3);
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec4,    r_attrib_type::dvec4);
SHOGLE_DECLARE_ATTRIB_TRAIT(dmat3,    r_attrib_type::dmat3);
SHOGLE_DECLARE_ATTRIB_TRAIT(dmat4,    r_attrib_type::dmat4);

SHOGLE_DECLARE_ATTRIB_TRAIT(int32,    r_attrib_type::i32);
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec2,    r_attrib_type::ivec2);
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec3,    r_attrib_type::ivec3);
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec4,    r_attrib_type::ivec4);

template<shader_attribute_type T>
constexpr r_push_constant r_format_pushconst(r_uniform uniform, const T& data) {
  return r_push_constant{
    .uniform = uniform,
    .data = {
      .data = std::addressof(data),
      .type = r_attrib_traits<T>::tag,
      .alignment = alignof(T),
      .size = sizeof(T),
    },
  };
}

constexpr uint32 BONE_TOMBSTONE = std::numeric_limits<uint32>::max();
template<size_t num_weights>
struct vertex_weights {
  vertex_weights() noexcept {
    for (uint32 i = 0; i < num_weights; ++i) {
      indices[i] = BONE_TOMBSTONE;
    }
  }

  uint32 indices[num_weights];
  float32 weights[num_weights];
};

template<size_t num_weights>
struct soa_vertices {
  std::vector<vec3> positions;
  std::vector<vec3> normals;
  std::vector<vec2> uvs;
  std::vector<vec3> tangents;
  std::vector<vec3> bitangents;
  std::vector<vertex_weights<num_weights>> weights;
  std::vector<color4> colors;
};

template<typename Vertex>
concept vert_has_positions = requires(Vertex vert, const vec3& pos) {
  vert.set_position(pos);
};

template<typename Vertex>
concept vert_has_normals = requires(Vertex vert, const vec3& norm) {
  vert.set_normal(norm);
};

template<typename Vertex>
concept vert_has_uvs = requires(Vertex vert, const vec2& uv) {
  vert.set_uv(uv);
};

template<typename Vertex>
concept vert_has_tangents = requires(Vertex vert, const vec3& tang, const vec3& bitang) {
  vert.set_tangent(tang);
  vert.set_bitangent(bitang);
};

template<typename Vertex, size_t num_weights>
concept vert_has_weights = requires(Vertex vert, const vertex_weights<num_weights>& weights) {
  vert.set_weights(weights);
};

template<typename Vertex>
concept vert_has_colors = requires(Vertex vert, const color4& color) {
  vert.set_color(color);
};

template<typename>
struct check_soa_vertex : public std::false_type {};

template<size_t w>
struct check_soa_vertex<soa_vertices<w>> : public std::true_type {};

template<typename T>
concept is_soa_vertex = check_soa_vertex<T>::value;

template<typename T>
concept is_aos_vertex = vert_has_positions<T>;

template<typename T>
concept is_vertex_type = is_soa_vertex<T> || is_aos_vertex<T>;

struct pn_vertex {
  vec3 position;
  vec3 normal;

  [[nodiscard]] static constexpr std::array<r_attrib_descriptor, 2> attrib_descriptor(
    const std::array<uint32, 2>& bindings = {0, 0}
  ) {
    std::array<r_attrib_descriptor, 2> desc;

    // layout (location = 0) in vec3 att_position;
    desc[0].binding = bindings[0];
    desc[0].location = 0;
    desc[0].type = r_attrib_type::vec3;
    desc[0].offset = offsetof(pn_vertex, position);

    // layout (location = 1) in vec3 att_normal;
    desc[1].binding = bindings[1];
    desc[1].location = 1;
    desc[1].type = r_attrib_type::vec3;
    desc[1].offset = offsetof(pn_vertex, normal);

    return desc;
  }

  void set_position(const vec3& pos) { position = pos; }
  void set_normal(const vec3& norm) { normal = norm; }
};
static_assert(
  is_aos_vertex<pn_vertex> &&
  vert_has_positions<pn_vertex> &&
  vert_has_normals<pn_vertex>
);

struct pnt_vertex {
  vec3 position;
  vec3 normal;
  vec2 uv;

  [[nodiscard]] static constexpr std::array<r_attrib_descriptor, 3> attrib_descriptor(
    const std::array<uint32, 3>& bindings = {0, 0, 0}
  ) {
    std::array<r_attrib_descriptor, 3> desc;

    // layout (location = 0) in vec3 att_position;
    desc[0].binding = bindings[0];
    desc[0].location = 0;
    desc[0].type = r_attrib_type::vec3;
    desc[0].offset = offsetof(pnt_vertex, position);

    // layout (location = 1) in vec3 att_normal;
    desc[1].binding = bindings[1];
    desc[1].location = 1;
    desc[1].type = r_attrib_type::vec3;
    desc[1].offset = offsetof(pnt_vertex, normal);

    // layout (location = 2) in vec2 att_uv;
    desc[2].binding = bindings[2];
    desc[2].location = 2;
    desc[2].type = r_attrib_type::vec2;
    desc[2].offset = offsetof(pnt_vertex, uv);

    return desc;
  }

  void set_position(const vec3& pos) { position = pos; }
  void set_normal(const vec3& norm) { normal = norm; }
  void set_uv(const vec2& uvs) { uv = uvs; }
};
static_assert(
  is_aos_vertex<pnt_vertex> &&
  vert_has_positions<pnt_vertex> &&
  vert_has_normals<pnt_vertex> &&
  vert_has_uvs<pnt_vertex>
);

} // namespace ntf

#undef SHOGLE_DECLARE_ATTRIB_TRAIT
