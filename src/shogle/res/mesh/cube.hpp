#pragma once

#include <shogle/render/gl/mesh.hpp>

namespace ntf::shogle::res {

class cube_mesh : public gl::mesh {
public:
  enum class type {
    texture2d,
    cubemap
  };

public:
  cube_mesh(type type);

private:
  type _type;
};


} // namespace ntf::shogle::res
