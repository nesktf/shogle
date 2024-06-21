#include <shogle/res/mesh/quad.hpp>

namespace {

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

namespace ntf::shogle {

quad::quad(type type) : _type(type) {
  using att_coords2d    = shader_attribute<0, vec2>;
  using att_texcoords2d = shader_attribute<1, vec2>;

  using att_coords3d    = shader_attribute<0, vec3>;
  using att_normals3d   = shader_attribute<1, vec3>;
  using att_texcoords3d = shader_attribute<2, vec2>;

  switch (type) {
    case type::normal2d: {
      _mesh.add_vertex_buffer(vert2d, sizeof(vert2d), att_coords2d{}, att_texcoords2d{});
      break;
    }
    case type::normal3d: {
      _mesh.add_vertex_buffer(vert3d, sizeof(vert3d), att_coords3d{}, att_normals3d{}, att_texcoords3d{});
      break;
    }
    case type::inverted2d: {
      _mesh.add_vertex_buffer(vert2d_inv, sizeof(vert2d_inv), att_coords2d{}, att_texcoords2d{});
      break;
    }
    case type::inverted3d: {
      _mesh.add_vertex_buffer(vert3d_inv, sizeof(vert3d_inv), att_coords3d{}, att_normals3d{}, att_texcoords3d{});
      break;
    }
  }
  _mesh.add_index_buffer(indices, sizeof(indices));
}

} // namespace ntf::shogle
