#pragma once

#include "core/texture.hpp"
#include "resources/model_data.hpp"

namespace ntf::shogle {

class Model {
public:
  class Mesh {
  public:
    Mesh(const ModelData::MeshData& mesh);
    ~Mesh();

    Mesh(Mesh&&) = default;
    Mesh& operator=(Mesh&&) = default;

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

  public:
    GLuint id(void) const { return vao; }
    size_t ind(void) const { return indices; }

  public:
    std::vector<Texture> tex;

  private:
    GLuint vao, vbo, ebo;
    size_t indices;
  };

public:
  Model(std::unique_ptr<ModelData> data);
  ~Model() = default;

  Model(Model&&) = default;
  Model& operator=(Model&&) = default;

  Model(const Model&) = delete;
  Model& operator=(const Model&) = delete;

public:
  std::vector<Mesh> meshes;
};


}
