#pragma once

#include <shogle/core/types.hpp>
#include <shogle/resources/loader/texture.hpp>

#include <vector>

namespace ntf::res {

struct model_loader {
  struct mesh {
    struct material {
      texture_loader texture;
      std::string uniform_name;
    };
    struct vertex {
      vec3 coord;
      vec3 normal;
      vec2 tex_coord;
    };
    std::vector<vertex> vertices;
    std::vector<uint> indices;
    std::vector<material> materials;
  };

  model_loader(std::string _path);

  std::string path;
  std::vector<mesh> meshes;
};


} // namespace ntf::res
