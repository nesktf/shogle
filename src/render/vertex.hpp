#pragma once

#include "./types.hpp"

namespace ntf {

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

  [[nodiscard]] static constexpr r_attrib_binding attrib_binding() {
    return r_attrib_binding{
      .binding = 0,
      .stride = sizeof(pn_vertex),
    };
  }
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

  [[nodiscard]] static constexpr r_attrib_binding attrib_binding() {
    return r_attrib_binding {
      .binding = 0,
      .stride = sizeof(pnt_vertex),
    };
  }
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
