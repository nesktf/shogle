#pragma once

#include <shogle/render/gl/mesh.hpp>

namespace ntf::shogle {

class quad_mesh : public gl::mesh {
public:
  enum class type {
    normal2d,
    inverted2d,
    normal3d,
    inverted3d
  };

public:
  quad_mesh(type type = type::normal2d);

private:
  type _type;
};

} // namespace ntf::shogle
