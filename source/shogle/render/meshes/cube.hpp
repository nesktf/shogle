#pragma once

#include <shogle/render/gl/mesh.hpp>

namespace ntf::shogle::meshes {

class cube : public gl::mesh {
public:
  enum class type {
    texture2d,
    cubemap
  };

public:
  cube(type type);

private:
  type _type;
};


} // namespace ntf::shogle::meshes
