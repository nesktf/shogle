#pragma once

#include <shogle/render/mesh.hpp>

namespace ntf::shogle {

using shadatt_coords2d    = shader_attribute<0, vec2>;
using shadatt_texcoords2d = shader_attribute<1, vec2>;

using shadatt_coords3d    = shader_attribute<0, vec3>;
using shadatt_normals3d   = shader_attribute<1, vec3>;
using shadatt_texcoords3d = shader_attribute<2, vec2>;

using shadatt_coordscmap  = shader_attribute<0, vec3>;

enum class quad_type {
  normal2d,
  inverted2d,
  normal3d,
  inverted3d
};

mesh load_quad(quad_type type);

enum class cube_type {
  texture2d,
  cubemap
};

mesh load_cube(cube_type type);

} // namespace ntf::shogle
