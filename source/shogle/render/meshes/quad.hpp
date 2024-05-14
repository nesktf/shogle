#pragma once

#include <shogle/render/gl/mesh.hpp>

namespace ntf::shogle::meshes {

class quad : public gl::mesh {
public:
  enum class type {
    normal2d,
    inverted2d,
    normal3d,
    inverted3d
  };

public:
  quad(type type);

private:
  type _type;
};

} // namespace ntf::shogle::meshes
