#pragma once

#include "resources/texture_data.hpp"
#include "glm/glm.hpp"
#include <assimp/scene.h>
#include <vector>

namespace ntf::shogle {

class ModelData {
public:
  struct Vertex {
    glm::vec3 ver_coord;
    glm::vec3 ver_norm;
    glm::vec2 tex_coord;
  };
  struct MeshData {
    std::vector<Vertex> vert;
    std::vector<GLuint> ind;
    std::vector<TextureData> tex;
  };

public:
  ModelData(const char* path);
  ~ModelData() = default;

public:
  std::vector<MeshData> meshes;
};



}
