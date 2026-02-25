#pragma once

#include <shogle/render/common.hpp>

namespace shogle {

enum class vertex_layout_mode {
  soa,
  aos,
};

// clang-format off
template<vertex_layout_mode Mode>
constexpr auto make_pn_unindexed_cube() {
  static_assert(Mode == vertex_layout_mode::soa);
return std::to_array<f32>({
  // position             // normal
  -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,
   0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f, 
   0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f, 
   0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f, 
  -0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f, 
  -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f, 

  -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
   0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
   0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
   0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
  -0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
  -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,

  -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,
  -0.5f,  0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,
  -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,
  -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,
  -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,
  -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,

   0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,
   0.5f,  0.5f, -0.5f,    1.0f,  0.0f,  0.0f,
   0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,
   0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,
   0.5f, -0.5f,  0.5f,    1.0f,  0.0f,  0.0f,
   0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,

  -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,
   0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,
   0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,
   0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,
  -0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,
  -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,

  -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,
   0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,
   0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,
   0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,
  -0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,
  -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f
});
}

template<vertex_layout_mode Mode>
constexpr auto make_pnt_unindexed_cube() {
  static_assert(Mode == vertex_layout_mode::soa);
return std::to_array<f32>({
  // position             // normal             // uv
  -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
   0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
   0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
   0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
  -0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   0.0f, 1.0f,
  -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   0.0f, 0.0f,

  -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
   0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
   0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
   0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
  -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   0.0f, 0.0f,

  -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
  -0.5f,  0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
  -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
  -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
  -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
  -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

   0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
   0.5f,  0.5f, -0.5f,    1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
   0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
   0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
   0.5f, -0.5f,  0.5f,    1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
   0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

  -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
   0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
   0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
   0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
  -0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
  -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,   0.0f, 1.0f,

  -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
   0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
   0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
   0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
  -0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,   0.0f, 0.0f,
  -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
});
}

template<vertex_layout_mode Mode, bool InvUv = false>
constexpr auto make_pnt_indexed_quad() {
  static_assert(Mode == vertex_layout_mode::soa);
	if constexpr (InvUv) {
return std::to_array<f32>({
  // position             // normal             // uv
  -0.5f, -0.5f,  0.0f,    0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
   0.5f, -0.5f,  0.0f,    0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
   0.5f,  0.5f,  0.0f,    0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
  -0.5f,  0.5f,  0.0f,    0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
});
	} else {
return std::to_array<f32>({
  // position             // normal             // uv
  -0.5f, -0.5f,  0.0f,    0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
   0.5f, -0.5f,  0.0f,    0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
   0.5f,  0.5f,  0.0f,    0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
  -0.5f,  0.5f,  0.0f,    0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
});
}

}
constexpr inline auto pnt_indexed_quad_ind_u32 = std::to_array<u32>({
  0, 1, 2, // bottom right triangle
  0, 2, 3  // top left triangle
});
// clang-format on

template<u32 TexExtent>
constexpr auto make_missing_albedo() -> std::array<u8, 4u * TexExtent * TexExtent> {
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
};

} // namespace shogle
