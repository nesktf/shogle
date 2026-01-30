#pragma once

#include <shogle/render/common.hpp>

namespace shogle {

namespace impl {

template<bool UseAos, typename T>
consteval auto make_pnt_layout() {
  static_assert(T::attribute_count == 3u);
  return vertex_layout<3u>{
    .attribs = {{
      {.location = 0, .type = attribute_type::vec3, .offset = UseAos * offsetof(T, pos)},
      {.location = 1, .type = attribute_type::vec3, .offset = UseAos * offsetof(T, normal)},
      {.location = 2, .type = attribute_type::vec2, .offset = UseAos * offsetof(T, uv)},
    }},
    .stride = UseAos * sizeof(T),
  };
}

template<bool UseAos, typename T>
consteval auto make_pn_layout() {
  static_assert(T::attribute_count == 2u);
  return vertex_layout<2u>{
    .attribs = {{
      {.location = 0, .type = attribute_type::vec3, .offset = UseAos * offsetof(T, pos)},
      {.location = 1, .type = attribute_type::vec3, .offset = UseAos * offsetof(T, normal)},
    }},
    .stride = UseAos * sizeof(T),
  };
}

} // namespace impl

// clang-format off
struct aos_pnt_vertex {
public:
  static constexpr u32 attribute_count = 3;
	static constexpr inline vertex_layout<attribute_count> attributes();

public:
  vec3 pos;
  vec3 normal;
  vec2 uv;
};

constexpr inline auto aos_pnt_vertex::attributes() -> vertex_layout<attribute_count> {
	return impl::make_pnt_layout<true, aos_pnt_vertex>();
}

static_assert(meta::vertex_type<aos_pnt_vertex>);

struct soa_pnt_vertex {
public:
  static constexpr u32 attribute_count = 3;
	static constexpr inline vertex_layout<attribute_count> attributes();

public:
  vec3 pos;
  vec3 normal;
  vec2 uv;
};

constexpr inline auto soa_pnt_vertex::attributes() -> vertex_layout<attribute_count> {
	return impl::make_pnt_layout<false, soa_pnt_vertex>();
}

static_assert(meta::vertex_type<soa_pnt_vertex>);

struct aos_pn_vertex {
public:
  static constexpr u32 attribute_count = 2;
	static constexpr inline vertex_layout<attribute_count> attributes();

public:
  vec3 pos;
  vec3 normal;
};

constexpr inline auto aos_pn_vertex::attributes() -> vertex_layout<attribute_count> {
	return impl::make_pn_layout<true, aos_pn_vertex>();
}

static_assert(meta::vertex_type<aos_pn_vertex>);

struct soa_pn_vertex {
public:
  static constexpr u32 attribute_count = 2;
	static constexpr inline vertex_layout<attribute_count> attributes();

public:
  vec3 pos;
  vec3 normal;
};

constexpr inline auto soa_pn_vertex::attributes() -> vertex_layout<attribute_count> {
	return impl::make_pn_layout<false, soa_pn_vertex>();
}

static_assert(meta::vertex_type<soa_pn_vertex>);

template<ntf::meta::same_as_any<soa_pn_vertex, aos_pn_vertex> T>
constexpr inline auto pn_unindexed_cube_vert = std::to_array<T>({
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

template<ntf::meta::same_as_any<soa_pnt_vertex, aos_pnt_vertex> T>
constexpr inline auto pnt_unindexed_cube_vert = std::to_array<T>({
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

template<ntf::meta::same_as_any<soa_pnt_vertex, aos_pnt_vertex> T>
constexpr inline auto pnt_indexed_quad_vert = std::to_array<T>({
  // position               // normal                // uv
  {{-0.5f, -0.5f,  0.0f},   { 0.0f,  0.0f,  1.0f},   {0.0f, 0.0f}},
  {{ 0.5f, -0.5f,  0.0f},   { 0.0f,  0.0f,  1.0f},   {1.0f, 0.0f}},
  {{ 0.5f,  0.5f,  0.0f},   { 0.0f,  0.0f,  1.0f},   {1.0f, 1.0f}},
  {{-0.5f,  0.5f,  0.0f},   { 0.0f,  0.0f,  1.0f},   {0.0f, 1.0f}},
});

template<ntf::meta::same_as_any<soa_pnt_vertex, aos_pnt_vertex> T>
constexpr inline auto pnt_indexed_quad_vert_inv = std::to_array<T>({
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
