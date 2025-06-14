#pragma once

#include "./pipeline.hpp"

namespace ntf::render {

constexpr uint32 BONE_TOMBSTONE = std::numeric_limits<uint32>::max();
template<size_t num_weights>
struct vertex_weights {
  vertex_weights() noexcept {
    for (uint32 i = 0; i < num_weights; ++i) {
      indices[i] = BONE_TOMBSTONE;
    }
  }

  uint32 indices[num_weights];
  f32 weights[num_weights];
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

} // namespace ntf::render

namespace ntf::meta {

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
concept vert_has_weights = requires(Vertex vert, const ntfr::vertex_weights<num_weights>& weight) {
  vert.set_weights(weight);
};

template<typename Vertex>
concept vert_has_colors = requires(Vertex vert, const color4& color) {
  vert.set_color(color);
};

template<typename>
struct check_soa_vertex : public std::false_type {};

template<size_t w>
struct check_soa_vertex<ntfr::soa_vertices<w>> : public std::true_type {};

template<typename T>
concept is_soa_vertex = check_soa_vertex<T>::value;

template<typename T>
concept is_aos_vertex = vert_has_positions<T>;

template<typename T>
concept is_vertex_type = is_soa_vertex<T> || is_aos_vertex<T>;

} // namespacen ntf::meta

namespace ntf::render {

struct pn_vertex {
  vec3 position;
  vec3 normal;

  [[nodiscard]] static constexpr std::array<attribute_binding, 2u> aos_binding() {
    // [ pos_0, norm_0 | pos_1, norm_1 | ... | pos_n-1, norm_n-1 ]
    std::array<attribute_binding, 2> desc;

    // layout (location = 0) in vec3 att_position;
    desc[0].type = attribute_type::vec3;
    desc[0].offset = offsetof(pn_vertex, position);
    desc[0].stride = sizeof(pn_vertex);
    desc[0].location = 0;

    // layout (location = 1) in vec3 att_normal;
    desc[1].type = attribute_type::vec3;
    desc[1].offset = offsetof(pn_vertex, normal);
    desc[1].stride = sizeof(pn_vertex);
    desc[1].location = 1;

    return desc;
  }

  [[nodiscard]] static constexpr std::array<attribute_binding, 2u> soa_binding(
    size_t vertex_count
  ) {
    // [ pos_0, pos_1, ..., pos_n-1 | norm_0, norm_1, ..., norm_n-1 ]
    // vertex_count == n
    std::array<attribute_binding, 2> desc;

    // layout (location = 0) in vec3 att_position;
    constexpr size_t pos_stride = 3*meta::attribute_size(attribute_type::vec3);
    desc[0].type = attribute_type::vec3;
    desc[0].offset = pos_stride*vertex_count;
    desc[0].stride = pos_stride;
    desc[0].location = 0;

    // layout (location = 1) in vec3 att_normal;
    constexpr size_t norm_stride = 3*meta::attribute_size(attribute_type::vec3);
    desc[1].type = attribute_type::vec3;
    desc[1].offset = norm_stride*vertex_count;
    desc[1].stride = norm_stride;
    desc[1].location = 1;

    return desc;
  }

  void set_position(const vec3& pos) { position = pos; }
  void set_normal(const vec3& norm) { normal = norm; }
};
static_assert(
  meta::is_aos_vertex<pn_vertex> &&
  meta::vert_has_positions<pn_vertex> &&
  meta::vert_has_normals<pn_vertex>
);

struct pnt_vertex {
  vec3 position;
  vec3 normal;
  vec2 uv;

  [[nodiscard]] static constexpr std::array<attribute_binding, 3u> aos_binding() {
    // [ pos_0, norm_0, uv_0 | pos_1, norm_1, uv_1 | ... | pos_n-1, norm_n-1, uv_n-1 ]
    std::array<attribute_binding, 3u> desc;

    // layout (location = 0) in vec3 att_position;
    desc[0].type = attribute_type::vec3;
    desc[0].offset = offsetof(pnt_vertex, position);
    desc[0].stride = sizeof(pnt_vertex);
    desc[0].location = 0;

    // layout (location = 1) in vec3 att_normal;
    desc[1].type = attribute_type::vec3;
    desc[1].offset = offsetof(pnt_vertex, normal);
    desc[1].stride = sizeof(pnt_vertex);
    desc[1].location = 1;

    // layout (location = 2) in vec2 att_uv;
    desc[2].type = attribute_type::vec2;
    desc[2].offset = offsetof(pnt_vertex, uv);
    desc[2].stride = sizeof(pnt_vertex);
    desc[2].location = 2;

    return desc;
  }

  [[nodiscard]] static constexpr std::array<attribute_binding, 3u> soa_binding(
    size_t vertex_count
  ) {
    // [ pos_0, pos_1, ..., pos_n-1 | norm_0, norm_1, ..., norm_n-1 | uv_0, uv_1, ..., uv_n-1 ]
    // vertex_count == n
    std::array<attribute_binding, 3u> desc;

    // layout (location = 0) in vec3 att_position;
    constexpr size_t pos_stride = 3*meta::attribute_size(attribute_type::vec3);
    desc[0].type = attribute_type::vec3;
    desc[0].offset = pos_stride*vertex_count;
    desc[0].stride = 0;
    desc[0].location = 0;

    // layout (location = 1) in vec3 att_normal;
    constexpr size_t norm_stride = 3*meta::attribute_size(attribute_type::vec3);
    desc[1].type = attribute_type::vec3;
    desc[1].offset = norm_stride*vertex_count;
    desc[1].stride = 0;
    desc[1].location = 1;

    // layout (location = 2) in vec2 att_uv;
    constexpr size_t uv_stride = 2*meta::attribute_size(attribute_type::vec2);
    desc[2].type = attribute_type::vec2;
    desc[2].offset = uv_stride*vertex_count;
    desc[2].stride = 0;
    desc[2].location = 2;

    return desc;
  }

  void set_position(const vec3& pos) { position = pos; }
  void set_normal(const vec3& norm) { normal = norm; }
  void set_uv(const vec2& uvs) { uv = uvs; }
};
static_assert(
  meta::is_aos_vertex<pnt_vertex> &&
  meta::vert_has_positions<pnt_vertex> &&
  meta::vert_has_normals<pnt_vertex> &&
  meta::vert_has_uvs<pnt_vertex>
);

} // namespace ntf::render
