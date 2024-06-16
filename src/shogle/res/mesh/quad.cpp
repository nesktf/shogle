#include <shogle/res/meshes/quad.hpp>

namespace {

// struct quad2d_vert {
//   ntf::vec2 coord;
//   ntf::vec2 texcoord;
// };
//
// struct quad3d_vert {
//   ntf::vec3 coord;
//   ntf::vec3 normal;
//   ntf::vec2 texcoord;
// };
//
// quad2d_vert vert2d[] = {
//   {{-0.5f, -0.5f}, {0.0f, 0.0f}},
//   {{0.5f, -0.5f}, {1.0f, 0.0f}},
//   {{0.5f, 0.5f}, {1.0f, 1.0f}},
//   {{-0.5f, 0.5f}, {0.0f, 1.0f}}
// };

// inverted texture quads are considered "normal" for convenience
float vert2d[] = { // inverted in texture space (for stb_image textures)
  // coord        // texcoord
  -0.5f, -0.5f,   0.0f, 0.0f,
   0.5f, -0.5f,   1.0f, 0.0f,
   0.5f,  0.5f,   1.0f, 1.0f,
  -0.5f,  0.5f,   0.0f, 1.0f
};
float vert2d_inv[] = { // not inverted (for framebuffers)
  // coord        // texcoord
  -0.5f, -0.5f,   0.0f, 1.0f,
   0.5f, -0.5f,   1.0f, 1.0f,
   0.5f,  0.5f,   1.0f, 0.0f,
  -0.5f,  0.5f,   0.0f, 0.0f
};

float vert3d[] = { // inverted in texture space (for stb_image textures)
  // coord                // normal              // texcoord
  -0.5f, -0.5f,  0.0f,    0.0f,  0.0f,  1.0f,    0.0f,  0.0f,
   0.5f, -0.5f,  0.0f,    0.0f,  0.0f,  1.0f,    1.0f,  0.0f,
   0.5f,  0.5f,  0.0f,    0.0f,  0.0f,  1.0f,    1.0f,  1.0f,
  -0.5f,  0.5f,  0.0f,    0.0f,  0.0f,  1.0f,    0.0f,  1.0f
};
float vert3d_inv[] = { // not inverted (for framebuffers)
  // coord                // normal              // texcoord
  -0.5f, -0.5f,  0.0f,    0.0f,  0.0f,  1.0f,    0.0f,  1.0f,
   0.5f, -0.5f,  0.0f,    0.0f,  0.0f,  1.0f,    1.0f,  1.0f,
   0.5f,  0.5f,  0.0f,    0.0f,  0.0f,  1.0f,    1.0f,  0.0f,
  -0.5f,  0.5f,  0.0f,    0.0f,  0.0f,  1.0f,    0.0f,  0.0f
};

uint indices[] = {
  0, 1, 2, // bottom right triangle
  0, 2, 3  // top left triangle
};

}

namespace ntf::shogle::res {

quad_mesh::quad_mesh(type type) : _type(type) {
  using att_coords2d    = gl::shader_attribute<0, vec2>;
  using att_texcoords2d = gl::shader_attribute<1, vec2>;

  using att_coords3d    = gl::shader_attribute<0, vec3>;
  using att_normals3d   = gl::shader_attribute<1, vec3>;
  using att_texcoords3d = gl::shader_attribute<2, vec2>;

  switch (type) {
    case type::normal2d: {
      add_vertex_buffer(vert2d, sizeof(vert2d), att_coords2d{}, att_texcoords2d{});
      break;
    }
    case type::normal3d: {
      add_vertex_buffer(vert3d, sizeof(vert3d), att_coords3d{}, att_normals3d{}, att_texcoords3d{});
      break;
    }
    case type::inverted2d: {
      add_vertex_buffer(vert2d_inv, sizeof(vert2d_inv), att_coords2d{}, att_texcoords2d{});
      break;
    }
    case type::inverted3d: {
      add_vertex_buffer(vert3d_inv, sizeof(vert3d_inv), att_coords3d{}, att_normals3d{}, att_texcoords3d{});
      break;
    }
  }
  add_index_buffer(indices, sizeof(indices));
}

} // namespace ntf::shogle::res
