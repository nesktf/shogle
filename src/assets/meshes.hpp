#pragma once

#include "../stl/types.hpp"
#include "../render/render.hpp"

namespace ntf {

struct pn_vertex {
  vec3 position;
  vec3 normal;

  [[nodiscard]] static constexpr r_attrib_binding attrib_binding() {
    return r_attrib_binding{
      .binding = 0,
      .stride = sizeof(pn_vertex),
    };
  }
  [[nodiscard]] static constexpr std::array<r_attrib_descriptor, 2> attrib_descriptor() {
    std::array<r_attrib_descriptor, 2> desc;

    // layout (location = 0, binding = 0) in vec3 att_position;
    desc[0].binding = 0;
    desc[0].location = 0;
    desc[0].type = r_attrib_type::vec3;
    desc[0].offset = offsetof(pn_vertex, position);

    // layout (location = 1, binding = 0) in vec3 att_normal;
    desc[1].binding = 0;
    desc[1].location = 1;
    desc[1].type = r_attrib_type::vec3;
    desc[1].offset = offsetof(pn_vertex, normal);

    return desc;
  }
};

constexpr inline pn_vertex pn_unindexed_cube_vert[] {
  // position               // normal
  {{-0.5f, -0.5f, -0.5f},   { 0.0f,  0.0f, -1.0f}},
  {{ 0.5f, -0.5f, -0.5f},   { 0.0f,  0.0f, -1.0f}}, 
  {{ 0.5f,  0.5f, -0.5f},   { 0.0f,  0.0f, -1.0f}}, 
  {{ 0.5f,  0.5f, -0.5f},   { 0.0f,  0.0f, -1.0f}}, 
  {{-0.5f,  0.5f, -0.5f},   { 0.0f,  0.0f, -1.0f}}, 
  {{-0.5f, -0.5f, -0.5f},   { 0.0f,  0.0f, -1.0f}}, 

  {{-0.5f, -0.5f,  0.5f},   { 0.0f,  0.0f,  1.0f}},
  {{ 0.5f, -0.5f,  0.5f},   { 0.0f,  0.0f,  1.0f}},
  {{ 0.5f,  0.5f,  0.5f},   { 0.0f,  0.0f,  1.0f}},
  {{ 0.5f,  0.5f,  0.5f},   { 0.0f,  0.0f,  1.0f}},
  {{-0.5f,  0.5f,  0.5f},   { 0.0f,  0.0f,  1.0f}},
  {{-0.5f, -0.5f,  0.5f},   { 0.0f,  0.0f,  1.0f}},

  {{-0.5f,  0.5f,  0.5f},   {-1.0f,  0.0f,  0.0f}},
  {{-0.5f,  0.5f, -0.5f},   {-1.0f,  0.0f,  0.0f}},
  {{-0.5f, -0.5f, -0.5f},   {-1.0f,  0.0f,  0.0f}},
  {{-0.5f, -0.5f, -0.5f},   {-1.0f,  0.0f,  0.0f}},
  {{-0.5f, -0.5f,  0.5f},   {-1.0f,  0.0f,  0.0f}},
  {{-0.5f,  0.5f,  0.5f},   {-1.0f,  0.0f,  0.0f}},

  {{ 0.5f,  0.5f,  0.5f},   { 1.0f,  0.0f,  0.0f}},
  {{ 0.5f,  0.5f, -0.5f},   { 1.0f,  0.0f,  0.0f}},
  {{ 0.5f, -0.5f, -0.5f},   { 1.0f,  0.0f,  0.0f}},
  {{ 0.5f, -0.5f, -0.5f},   { 1.0f,  0.0f,  0.0f}},
  {{ 0.5f, -0.5f,  0.5f},   { 1.0f,  0.0f,  0.0f}},
  {{ 0.5f,  0.5f,  0.5f},   { 1.0f,  0.0f,  0.0f}},

  {{-0.5f, -0.5f, -0.5f},   { 0.0f, -1.0f,  0.0f}},
  {{ 0.5f, -0.5f, -0.5f},   { 0.0f, -1.0f,  0.0f}},
  {{ 0.5f, -0.5f,  0.5f},   { 0.0f, -1.0f,  0.0f}},
  {{ 0.5f, -0.5f,  0.5f},   { 0.0f, -1.0f,  0.0f}},
  {{-0.5f, -0.5f,  0.5f},   { 0.0f, -1.0f,  0.0f}},
  {{-0.5f, -0.5f, -0.5f},   { 0.0f, -1.0f,  0.0f}},

  {{-0.5f,  0.5f, -0.5f},   { 0.0f,  1.0f,  0.0f}},
  {{ 0.5f,  0.5f, -0.5f},   { 0.0f,  1.0f,  0.0f}},
  {{ 0.5f,  0.5f,  0.5f},   { 0.0f,  1.0f,  0.0f}},
  {{ 0.5f,  0.5f,  0.5f},   { 0.0f,  1.0f,  0.0f}},
  {{-0.5f,  0.5f,  0.5f},   { 0.0f,  1.0f,  0.0f}},
  {{-0.5f,  0.5f, -0.5f},   { 0.0f,  1.0f,  0.0f}}
};

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
  [[nodiscard]] static constexpr std::array<r_attrib_descriptor, 3> attrib_descriptor() {
    std::array<r_attrib_descriptor, 3> desc;

    // layout (location = 0, binding = 0) in vec3 att_position;
    desc[0].binding = 0;
    desc[0].location = 0;
    desc[0].type = r_attrib_type::vec3;
    desc[0].offset = offsetof(pnt_vertex, position);

    // layout (location = 1, binding = 0) in vec3 att_normal;
    desc[1].binding = 0;
    desc[1].location = 1;
    desc[1].type = r_attrib_type::vec3;
    desc[1].offset = offsetof(pnt_vertex, normal);

    // layout (location = 2, binding = 0) in vec2 att_uv;
    desc[2].binding = 0;
    desc[2].location = 2;
    desc[2].type = r_attrib_type::vec2;
    desc[2].offset = offsetof(pnt_vertex, uv);

    return desc;
  }
};

constexpr inline pnt_vertex pnt_unindexed_cube_vert[] {
  // position               // normal                // uv
  {{-0.5f, -0.5f, -0.5f},   { 0.0f,  0.0f, -1.0f},   {0.0f, 0.0f}},
  {{ 0.5f, -0.5f, -0.5f},   { 0.0f,  0.0f, -1.0f},   {1.0f, 0.0f}},
  {{ 0.5f,  0.5f, -0.5f},   { 0.0f,  0.0f, -1.0f},   {1.0f, 1.0f}},
  {{ 0.5f,  0.5f, -0.5f},   { 0.0f,  0.0f, -1.0f},   {1.0f, 1.0f}},
  {{-0.5f,  0.5f, -0.5f},   { 0.0f,  0.0f, -1.0f},   {0.0f, 1.0f}},
  {{-0.5f, -0.5f, -0.5f},   { 0.0f,  0.0f, -1.0f},   {0.0f, 0.0f}},

  {{-0.5f, -0.5f,  0.5f},   { 0.0f,  0.0f,  1.0f},   {0.0f, 0.0f}},
  {{ 0.5f, -0.5f,  0.5f},   { 0.0f,  0.0f,  1.0f},   {1.0f, 0.0f}},
  {{ 0.5f,  0.5f,  0.5f},   { 0.0f,  0.0f,  1.0f},   {1.0f, 1.0f}},
  {{ 0.5f,  0.5f,  0.5f},   { 0.0f,  0.0f,  1.0f},   {1.0f, 1.0f}},
  {{-0.5f,  0.5f,  0.5f},   { 0.0f,  0.0f,  1.0f},   {0.0f, 1.0f}},
  {{-0.5f, -0.5f,  0.5f},   { 0.0f,  0.0f,  1.0f},   {0.0f, 0.0f}},

  {{-0.5f,  0.5f,  0.5f},   {-1.0f,  0.0f,  0.0f},   {1.0f, 0.0f}},
  {{-0.5f,  0.5f, -0.5f},   {-1.0f,  0.0f,  0.0f},   {1.0f, 1.0f}},
  {{-0.5f, -0.5f, -0.5f},   {-1.0f,  0.0f,  0.0f},   {0.0f, 1.0f}},
  {{-0.5f, -0.5f, -0.5f},   {-1.0f,  0.0f,  0.0f},   {0.0f, 1.0f}},
  {{-0.5f, -0.5f,  0.5f},   {-1.0f,  0.0f,  0.0f},   {0.0f, 0.0f}},
  {{-0.5f,  0.5f,  0.5f},   {-1.0f,  0.0f,  0.0f},   {1.0f, 0.0f}},

  {{ 0.5f,  0.5f,  0.5f},   { 1.0f,  0.0f,  0.0f},   {1.0f, 0.0f}},
  {{ 0.5f,  0.5f, -0.5f},   { 1.0f,  0.0f,  0.0f},   {1.0f, 1.0f}},
  {{ 0.5f, -0.5f, -0.5f},   { 1.0f,  0.0f,  0.0f},   {0.0f, 1.0f}},
  {{ 0.5f, -0.5f, -0.5f},   { 1.0f,  0.0f,  0.0f},   {0.0f, 1.0f}},
  {{ 0.5f, -0.5f,  0.5f},   { 1.0f,  0.0f,  0.0f},   {0.0f, 0.0f}},
  {{ 0.5f,  0.5f,  0.5f},   { 1.0f,  0.0f,  0.0f},   {1.0f, 0.0f}},

  {{-0.5f, -0.5f, -0.5f},   { 0.0f, -1.0f,  0.0f},   {0.0f, 1.0f}},
  {{ 0.5f, -0.5f, -0.5f},   { 0.0f, -1.0f,  0.0f},   {1.0f, 1.0f}},
  {{ 0.5f, -0.5f,  0.5f},   { 0.0f, -1.0f,  0.0f},   {1.0f, 0.0f}},
  {{ 0.5f, -0.5f,  0.5f},   { 0.0f, -1.0f,  0.0f},   {1.0f, 0.0f}},
  {{-0.5f, -0.5f,  0.5f},   { 0.0f, -1.0f,  0.0f},   {0.0f, 0.0f}},
  {{-0.5f, -0.5f, -0.5f},   { 0.0f, -1.0f,  0.0f},   {0.0f, 1.0f}},

  {{-0.5f,  0.5f, -0.5f},   { 0.0f,  1.0f,  0.0f},   {0.0f, 1.0f}},
  {{ 0.5f,  0.5f, -0.5f},   { 0.0f,  1.0f,  0.0f},   {1.0f, 1.0f}},
  {{ 0.5f,  0.5f,  0.5f},   { 0.0f,  1.0f,  0.0f},   {1.0f, 0.0f}},
  {{ 0.5f,  0.5f,  0.5f},   { 0.0f,  1.0f,  0.0f},   {1.0f, 0.0f}},
  {{-0.5f,  0.5f,  0.5f},   { 0.0f,  1.0f,  0.0f},   {0.0f, 0.0f}},
  {{-0.5f,  0.5f, -0.5f},   { 0.0f,  1.0f,  0.0f},   {0.0f, 1.0f}}
};

constexpr inline pnt_vertex pnt_indexed_quad_vert[] {
  // position               // normal                // uv
  {{-0.5f, -0.5f,  0.0f},   { 0.0f,  0.0f,  1.0f},   {0.0f, 0.0f}},
  {{ 0.5f, -0.5f,  0.0f},   { 0.0f,  0.0f,  1.0f},   {1.0f, 0.0f}},
  {{ 0.5f,  0.5f,  0.0f},   { 0.0f,  0.0f,  1.0f},   {1.0f, 1.0f}},
  {{-0.5f,  0.5f,  0.0f},   { 0.0f,  0.0f,  1.0f},   {0.0f, 1.0f}},
};

constexpr inline pnt_vertex pnt_indexed_quad_vert_inv[] {
  // position               // normal                // uv
  {{-0.5f, -0.5f,  0.0f},   { 0.0f,  0.0f,  1.0f},   {0.0f, 1.0f}},
  {{ 0.5f, -0.5f,  0.0f},   { 0.0f,  0.0f,  1.0f},   {1.0f, 1.0f}},
  {{ 0.5f,  0.5f,  0.0f},   { 0.0f,  0.0f,  1.0f},   {1.0f, 0.0f}},
  {{-0.5f,  0.5f,  0.0f},   { 0.0f,  0.0f,  1.0f},   {0.0f, 0.0f}},
};

constexpr inline uint32 pnt_indexed_quad_ind[] {
  0, 1, 2, // bottom right triangle
  0, 2, 3  // top left triangle
};

} // namespace ntf
