#pragma once

#include <shogle/render/mesh.hpp>

namespace ntf::shogle {

class cube {
public:
  enum class type {
    texture2d,
    cubemap
  };

public:
  cube(type type);

public:
  class mesh& mesh() { return _mesh; }

private:
  class mesh _mesh{};
  type _type;
};


} // namespace ntf::shogle
