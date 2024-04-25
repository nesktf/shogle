#pragma once

#include <shogle/render/material.hpp>

#include <vector>

namespace ntf::render {

// Model3D::data_t
class ModelData {
public:
  struct Vertex {
    vec3 ver_coord;
    vec3 ver_norm;
    vec2 tex_coord;
  };
  struct MeshData {
    std::vector<Vertex> vert;
    std::vector<GLuint> ind;
    std::vector<Material::data_t> materials;
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
class Model {
public:
  using data_t = ModelData;

  class Mesh {
  public: // Resources can't be copied
    Mesh(const ModelData::MeshData& mesh);
    ~Mesh();

    Mesh(Mesh&&) noexcept;
    Mesh(const Mesh&) = delete;
    Mesh& operator=(Mesh&&) noexcept;
    Mesh& operator=(const Mesh&) = delete;

  public:
    GLuint id(void) const { return _vao; }
    size_t ind(void) const { return _indices; }

  public:
    std::vector<Material> materials;

  private:
    GLuint _vao, _vbo, _ebo;
    size_t _indices;
  };

public: // Resource wrappers also can't be copied
  Model(const Model::data_t* data);
  ~Model() = default;

  Model(Model&&) = default;
  Model(const Model&) = delete;
  Model& operator=(Model&&) = default;
  Model& operator=(const Model&) = delete;

public:
  std::vector<Mesh> meshes;
};

} // namespace ntf::render
