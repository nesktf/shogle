#pragma once

#include <shogle/res/material.hpp>

#include <vector>

namespace ntf {

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
class ModelRes {
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
  ModelRes(const ModelRes::data_t* data);
  ~ModelRes() = default;

  ModelRes(ModelRes&&) = default;
  ModelRes(const ModelRes&) = delete;
  ModelRes& operator=(ModelRes&&) = default;
  ModelRes& operator=(const ModelRes&) = delete;

public:
  std::vector<Mesh> meshes;
};

} // namespace ntf
