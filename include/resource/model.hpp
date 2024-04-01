#pragma once

#include "resource/texture.hpp"

#include <vector>

namespace ntf::shogle::res {

// Model::data_t
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
    std::vector<Texture::data_t> tex;
  };

public: // Resource data can be copied but i don't think is a good idea
  ModelData(std::string path);
  ~ModelData() = default;

  ModelData(ModelData&&) = default;
  ModelData& operator=(ModelData&&) = default;

  ModelData(const ModelData&) = delete;
  ModelData& operator=(const ModelData&) = delete;

public:
  std::vector<MeshData> meshes; 
};

// Model
class Model {
public:
  using data_t = ModelData;

  class Mesh {
  public: // Resources can't be copied
    Mesh(const ModelData::MeshData& mesh);
    ~Mesh();

    Mesh(Mesh&&) noexcept;
    Mesh& operator=(Mesh&&) noexcept;

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

public: // Resource wrappers also can't be copied
  Model(const Model::data_t* data);
  ~Model() = default;

  Model(Model&&) = default;
  Model& operator=(Model&&) = default;

  Model(const Model&) = delete;
  Model& operator=(const Model&) = delete;

public:
  std::vector<Mesh> meshes;
};

} // namespace ntf::shogle::res

