#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"
#include <assimp/scene.h>
#include <vector>

namespace ntf::shogle::res {

struct ModelData {
  struct Vertex {
    glm::vec3 coord;
    glm::vec3 normal;
    glm::vec2 tex_coord;
  };
  struct MeshTexture {
    std::string name;
    aiTextureType type;
  };
  struct MeshData {
    std::vector<Vertex> vert;
    std::vector<GLuint> ind;
    std::vector<MeshTexture> tex;
  };

  ModelData(const char* path);

  std::vector<MeshData> meshes;
};



}
