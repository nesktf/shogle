#include <shogle/res/mesh/cube.hpp>

namespace {

float cube_tex2d_vert[] = {
  // coord                 // normal             // tex_coord
  -0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,   0.0f,  0.0f,
   0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,   1.0f,  0.0f,
   0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,   1.0f,  1.0f,
   0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,   1.0f,  1.0f,
  -0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,   0.0f,  1.0f,
  -0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,   0.0f,  0.0f,

  -0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,   0.0f,  0.0f,
   0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,   1.0f,  0.0f,
   0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,   1.0f,  1.0f,
   0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,   1.0f,  1.0f,
  -0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,   0.0f,  1.0f,
  -0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,   0.0f,  0.0f,

  -0.5f,  0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,   1.0f,  0.0f,
  -0.5f,  0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,   1.0f,  1.0f,
  -0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,   0.0f,  1.0f,
  -0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,   0.0f,  1.0f,
  -0.5f, -0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,   0.0f,  0.0f,
  -0.5f,  0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,   1.0f,  0.0f,

   0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,   1.0f,  0.0f,
   0.5f,  0.5f, -0.5f,     1.0f,  0.0f,  0.0f,   1.0f,  1.0f,
   0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,   0.0f,  1.0f,
   0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,   0.0f,  1.0f,
   0.5f, -0.5f,  0.5f,     1.0f,  0.0f,  0.0f,   0.0f,  0.0f,
   0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,   1.0f,  0.0f,

  -0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,   0.0f,  1.0f,
   0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,   1.0f,  1.0f,
   0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,   1.0f,  0.0f,
   0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,   1.0f,  0.0f,
  -0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,   0.0f,  0.0f,
  -0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,   0.0f,  1.0f,

  -0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,   0.0f,  1.0f,
   0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,   1.0f,  1.0f,
   0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,   1.0f,  0.0f,
   0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,   1.0f,  0.0f,
  -0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,   0.0f,  0.0f,
  -0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,   0.0f,  1.0f
};

float cube_cmap_vert[] = {
  // just tex_coords
  -1.0f, -1.0f,  1.0f,
   1.0f, -1.0f,  1.0f,
   1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f, -1.0f,
  -1.0f,  1.0f,  1.0f,
   1.0f,  1.0f,  1.0f,
   1.0f,  1.0f, -1.0f,
  -1.0f,  1.0f, -1.0f
};
uint cube_cmap_ind[] = {
  // Right
  1, 2, 6,
  6, 5, 1,
  // Left
  0, 4, 7,
  7, 3, 0,
  // Top
  4, 5, 6,
  6, 7, 4,
  // Bottom
  0, 3, 2,
  2, 1, 0,
  // Back
  0, 1, 5,
  5, 4, 0,
  // Front
  3, 7, 6,
  6, 2, 3
};

}

namespace ntf::shogle {

cube::cube(type type) : _type(type) {
  using att_coords = shader_attribute<0, vec3>;
  using att_normals = shader_attribute<1, vec3>;
  using att_texcoords = shader_attribute<2, vec2>;

  using att_coordscmap = shader_attribute<0, vec3>;

  switch (type) {
    case type::texture2d: {
      _mesh.add_vertex_buffer(cube_tex2d_vert, sizeof(cube_tex2d_vert), att_coords{}, att_normals{}, att_texcoords{});
      break;
    }
    case type::cubemap: {
      _mesh.add_vertex_buffer(cube_cmap_vert, sizeof(cube_cmap_vert), att_coordscmap{});
      _mesh.add_index_buffer(cube_cmap_ind, sizeof(cube_cmap_ind));
      break;
    }
  }
}


} // namespace ntf::shogle
