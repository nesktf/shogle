#pragma once

#include "core/texture.hpp"
#include "resources/model_data.hpp"

namespace ntf::shogle {

class ModelMeshes {
public:
  class Mesh {
  public:
    Mesh(const ModelData::MeshData& mesh);

    GLuint id(void) const { return vao_id; }
    size_t ind(void) const { return indices; }

    std::vector<Texture> tex;

  private:
    GLuint vao_id, vbo_id, ebo_id;
    size_t indices;
    
  };
  ModelMeshes(const ModelData* data);

  std::vector<Mesh> meshes;
};


}
