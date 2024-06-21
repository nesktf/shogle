#pragma once

#include <shogle/render/mesh.hpp>

namespace ntf::shogle {

class quad {
public:
  enum class type {
    normal2d,
    inverted2d,
    normal3d,
    inverted3d
  };

public:
  quad(type type = type::normal2d);

public:
  class mesh& mesh() { return _mesh; }

private:
  class mesh _mesh{};
  type _type;
};

} // namespace ntf::shogle
