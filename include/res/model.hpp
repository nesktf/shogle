#pragma once

#include "res/texture.hpp"

#include <vector>

namespace ntf {

// Model3D::data_t
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

// Model3D
class ModelRes {
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
  ModelRes(const ModelRes::data_t* data);
  ~ModelRes() = default;

  ModelRes(ModelRes&&) = default;
  ModelRes& operator=(ModelRes&&) = default;

  ModelRes(const ModelRes&) = delete;
  ModelRes& operator=(const ModelRes&) = delete;

public:
  std::vector<Mesh> meshes;
};

} // namespace ntf
