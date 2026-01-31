#pragma once

#include <shogle/render/common.hpp>

namespace shogle {

// clang-format off
struct pnt_vertex {
public:
  static constexpr u32 attribute_count = 3u;
	static constexpr inline auto attributes() noexcept;

public:
  vec3 pos;
  vec3 normal;
  vec2 uv;
};

constexpr inline auto pnt_vertex::attributes() noexcept {
  return std::to_array<vertex_attribute>({
		{.location = 0, .type = attribute_type::vec3, .offset = offsetof(pnt_vertex, pos)},
		{.location = 1, .type = attribute_type::vec3, .offset = offsetof(pnt_vertex, normal)},
		{.location = 2, .type = attribute_type::vec2, .offset = offsetof(pnt_vertex, uv)},
	});
}

static_assert(meta::vertex_type<pnt_vertex>);

struct pn_vertex {
public:
  static constexpr u32 attribute_count = 2u;
	static constexpr inline auto attributes() noexcept;

public:
  vec3 pos;
  vec3 normal;
};

constexpr inline auto pn_vertex::attributes() noexcept{
	return std::to_array<vertex_attribute>({
    {.location = 0, .type = attribute_type::vec3, .offset = offsetof(pn_vertex, pos)},
		{.location = 1, .type = attribute_type::vec3, .offset = offsetof(pn_vertex, normal)},
  });
}

static_assert(meta::vertex_type<pn_vertex>);

struct pc_vertex {
public:
	static constexpr u32 attribute_count = 2u;
	static constexpr inline auto attributes() noexcept;

public:
	vec3 pos;
	vec4 color;
};

constexpr inline auto pc_vertex::attributes() noexcept{
	return std::to_array<vertex_attribute>({
    {.location = 0, .type = attribute_type::vec3, .offset = offsetof(pc_vertex, pos)},
		{.location = 1, .type = attribute_type::vec4, .offset = offsetof(pc_vertex, color)},
  });
}

constexpr inline auto pn_unindexed_cube_vert = std::to_array<pn_vertex>({
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
});

constexpr inline auto pnt_unindexed_cube_vert = std::to_array<pnt_vertex>({
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
});

constexpr inline auto pnt_indexed_quad_vert = std::to_array<pnt_vertex>({
  // position               // normal                // uv
  {{-0.5f, -0.5f,  0.0f},   { 0.0f,  0.0f,  1.0f},   {0.0f, 0.0f}},
  {{ 0.5f, -0.5f,  0.0f},   { 0.0f,  0.0f,  1.0f},   {1.0f, 0.0f}},
  {{ 0.5f,  0.5f,  0.0f},   { 0.0f,  0.0f,  1.0f},   {1.0f, 1.0f}},
  {{-0.5f,  0.5f,  0.0f},   { 0.0f,  0.0f,  1.0f},   {0.0f, 1.0f}},
});

constexpr inline auto pc_indexed_quad_vert = std::to_array<pc_vertex>({
  // position               // color
  {{-0.5f, -0.5f,  0.0f},   { 0.0f,  0.0f,  0.0f,  1.0f}},
});

constexpr inline auto pnt_indexed_quad_vert_inv = std::to_array<pnt_vertex>({
  // position               // normal                // uv
  {{-0.5f, -0.5f,  0.0f},   { 0.0f,  0.0f,  1.0f},   {0.0f, 1.0f}},
  {{ 0.5f, -0.5f,  0.0f},   { 0.0f,  0.0f,  1.0f},   {1.0f, 1.0f}},
  {{ 0.5f,  0.5f,  0.0f},   { 0.0f,  0.0f,  1.0f},   {1.0f, 0.0f}},
  {{-0.5f,  0.5f,  0.0f},   { 0.0f,  0.0f,  1.0f},   {0.0f, 0.0f}},
});

constexpr inline auto pnt_indexed_quad_ind_u32 = std::to_array<u32>({
  0, 1, 2, // bottom right triangle
  0, 2, 3  // top left triangle
});
// clang-format on

template<u32 TexExtent>
constexpr auto missing_albedo_bitmap = [] {
  std::array<u8, 4u * TexExtent * TexExtent> out;
  const u8 pixels[]{
    0x00, 0x00, 0x00, 0xFF, // black
    0xFE, 0x00, 0xFE, 0xFF, // pink
    0x00, 0x00, 0x00, 0xFF, // black again
  };

  for (u32 i = 0; i < TexExtent; ++i) {
    const u8* start = i % 2 == 0 ? &pixels[0] : &pixels[4]; // Start row with a different color
    u32 pos = 0;
    for (u32 j = 0; j < TexExtent; ++j) {
      pos = (pos + 4) % 8;
      for (u32 k = 0; k < 4; ++k) {
        out[(4 * i * TexExtent) + (4 * j) + k] = start[pos + k];
      }
    }
  }

  return out;
}();

} // namespace shogle
