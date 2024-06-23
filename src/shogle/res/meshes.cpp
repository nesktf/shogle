#include <shogle/res/meshes.hpp>

static float cube_tex2d_vert[] {
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

static float cube_cmap_vert[] {
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

static uint cube_cmap_ind[] {
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

// inverted texture quads are considered "normal" for convenience
static float quad2d_vert[] { // inverted in texture space (for stb_image textures)
  // coord        // texcoord
  -0.5f, -0.5f,   0.0f, 0.0f,
   0.5f, -0.5f,   1.0f, 0.0f,
   0.5f,  0.5f,   1.0f, 1.0f,
  -0.5f,  0.5f,   0.0f, 1.0f
};

static float quad2d_vert_inv[] { // not inverted (for framebuffers)
  // coord        // texcoord
  -0.5f, -0.5f,   0.0f, 1.0f,
   0.5f, -0.5f,   1.0f, 1.0f,
   0.5f,  0.5f,   1.0f, 0.0f,
  -0.5f,  0.5f,   0.0f, 0.0f
};

static float quad3d_vert[] { // inverted in texture space (for stb_image textures)
  // coord                // normal              // texcoord
  -0.5f, -0.5f,  0.0f,    0.0f,  0.0f,  1.0f,    0.0f,  0.0f,
   0.5f, -0.5f,  0.0f,    0.0f,  0.0f,  1.0f,    1.0f,  0.0f,
   0.5f,  0.5f,  0.0f,    0.0f,  0.0f,  1.0f,    1.0f,  1.0f,
  -0.5f,  0.5f,  0.0f,    0.0f,  0.0f,  1.0f,    0.0f,  1.0f
};

static float quad3d_vert_inv[] { // not inverted (for framebuffers)
  // coord                // normal              // texcoord
  -0.5f, -0.5f,  0.0f,    0.0f,  0.0f,  1.0f,    0.0f,  1.0f,
   0.5f, -0.5f,  0.0f,    0.0f,  0.0f,  1.0f,    1.0f,  1.0f,
   0.5f,  0.5f,  0.0f,    0.0f,  0.0f,  1.0f,    1.0f,  0.0f,
  -0.5f,  0.5f,  0.0f,    0.0f,  0.0f,  1.0f,    0.0f,  0.0f
};

static uint quad_ind[] {
  0, 1, 2, // bottom right triangle
  0, 2, 3  // top left triangle
};

namespace ntf::shogle {

mesh load_cube(cube_type type) {
  mesh mesh;

  switch (type) {
    case cube_type::texture2d: {
      mesh.add_vertex_buffer(cube_tex2d_vert, sizeof(cube_tex2d_vert),
        shadatt_coords3d{}, shadatt_normals3d{}, shadatt_texcoords3d{}
      );
      break;
    }
    case cube_type::cubemap: {
      mesh.add_vertex_buffer(cube_cmap_vert, sizeof(cube_cmap_vert),
        shadatt_coordscmap{}
      );
      mesh.add_index_buffer(cube_cmap_ind, sizeof(cube_cmap_ind));
    }
  }

  return mesh;
}

mesh load_quad(quad_type type) {
  mesh mesh;

  switch (type) {
    case quad_type::normal2d: {
      mesh.add_vertex_buffer(quad2d_vert, sizeof(quad2d_vert), 
        shadatt_coords2d{}, shadatt_texcoords2d{}
      );
      break;
    }
    case quad_type::normal3d: {
      mesh.add_vertex_buffer(quad3d_vert, sizeof(quad3d_vert),
        shadatt_coords3d{}, shadatt_normals3d{}, shadatt_texcoords3d{}
      );
      break;
    }
    case quad_type::inverted2d: {
      mesh.add_vertex_buffer(quad2d_vert_inv, sizeof(quad2d_vert_inv), 
        shadatt_coords2d{}, shadatt_texcoords2d{}
      );
      break;
    }
    case quad_type::inverted3d: {
      mesh.add_vertex_buffer(quad3d_vert_inv, sizeof(quad3d_vert_inv),
        shadatt_coords3d{}, shadatt_normals3d{}, shadatt_texcoords3d{}
      );
      break;
    }
  }
  mesh.add_index_buffer(quad_ind, sizeof(quad_ind));

  return mesh;
}

} // namespace ntf::shogle
