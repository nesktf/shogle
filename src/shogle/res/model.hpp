#pragma once

#include <shogle/render/mesh.hpp>

#include <shogle/res/texture.hpp>
#include <shogle/res/util.hpp>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace ntf {

enum class material_type {
  diffuse = 0,
  specular
};

struct mesh_data {
  struct vertex {
    vec3 coord;
    vec3 normal;
    vec2 tex_coord;
  };

  std::string name;
  std::vector<vertex> vertices;
  std::vector<uint> indices;
  pair_vector<material_type, texture2d_data> materials;
};

class model {
public:
  class textured_mesh {
  public:
    textured_mesh(mesh mesh, std::unordered_map<material_type, texture2d> materials) :
      _mesh(std::move(mesh)), _materials(std::move(materials)) {}

  public:
    const mesh& get_mesh() const { return _mesh; }
    const texture2d& operator[](material_type material) const { return _materials.at(material); }

    auto cbegin() const { return _materials.cbegin(); }
    auto cend() const { return _materials.cend(); }
    auto begin() { return _materials.begin(); }
    auto end() { return _materials.end(); }

    size_t size() const { return _materials.size(); }

  private:
    mesh _mesh;
    std::unordered_map<material_type, texture2d> _materials;
  };

public:
  model() = default;
  model(std::vector<mesh_data> meshes);

public:
  const textured_mesh& operator[](std::string_view name) const { return _meshes.at(name.data()); }
  const textured_mesh& at(std::string_view name) const { return _meshes.at(name.data()); }

  auto cbegin() const { return _meshes.cbegin(); }
  auto cend() const { return _meshes.cend(); }
  auto begin() { return _meshes.begin(); }
  auto end() { return _meshes.end(); }

  size_t size() const { return _meshes.size(); }

private:
  std::unordered_map<std::string, textured_mesh> _meshes;
};

struct model_data {
public:
  struct loader {
    model operator()(model_data data) { return model{std::move(data.meshes)}; }
  };

public:
  model_data(std::string_view path_, tex_filter filter_, tex_wrap wrap_);

public:
  std::vector<mesh_data> meshes;
};


inline model_data::model_data(std::string_view path_, tex_filter filter_, tex_wrap wrap_) {
  Assimp::Importer import;
  const aiScene* scene = import.ReadFile(path_.data(), aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    throw ntf::error{"[ntf::model_data] ASSIMP error: {}", import.GetErrorString()};
  }

  auto dir = file_dir(path_.data());
  std::vector<std::string> loaded_materials;

  auto load_material = [&](mesh_data& mesh, aiMaterial* aimat, aiTextureType aitype) {
    material_type mat_type;
    switch (aitype) {
      case aiTextureType_SPECULAR: {
        mat_type = material_type::specular;
        break;
      }
      default: {
        mat_type = material_type::diffuse;
        break;
      }
    }

    for (size_t i = 0; i < aimat->GetTextureCount(aitype); ++i) {
      aiString filename;
      aimat->GetTexture(aitype, i, &filename);
      auto tex_path = dir + "/" + std::string{filename.C_Str()};
      bool skip {false};

      for (const auto& mat_path : loaded_materials) {
        if (std::strcmp(mat_path.data(), tex_path.data()) == 0) {
          skip = true;
          break;
        }
      }

      if (!skip) {
        mesh.materials.emplace_back(std::make_pair(
          mat_type, texture2d_data{tex_path, filter_, wrap_}
        ));
        loaded_materials.emplace_back(std::move(tex_path));
      }
    }
  };

  for (size_t i = 0; i < scene->mNumMeshes; ++i) {
    mesh_data mesh;
    aiMesh* curr_aimesh = scene->mMeshes[i];

    // Extract vertices
    for (size_t j = 0; j < curr_aimesh->mNumVertices; ++j) { 
      mesh_data::vertex vert;
      vert.coord = vec3{
        curr_aimesh->mVertices[j].x,
        curr_aimesh->mVertices[j].y,
        curr_aimesh->mVertices[j].z
      };
      vert.normal = vec3{
        curr_aimesh->mNormals[j].x,
        curr_aimesh->mNormals[j].y,
        curr_aimesh->mNormals[j].z
      };

      if (curr_aimesh->mTextureCoords[0]) {
        vert.tex_coord = vec2{
          curr_aimesh->mTextureCoords[0][j].x,
          curr_aimesh->mTextureCoords[0][j].y
        };
      }

      mesh.vertices.emplace_back(std::move(vert));
    }

    // Extract indices
    for (size_t j = 0; j < curr_aimesh->mNumFaces; ++j) {
      aiFace face = curr_aimesh->mFaces[j];
      for (size_t k = 0; k < face.mNumIndices; ++k) {
        mesh.indices.emplace_back(face.mIndices[k]);
      }
    }

    // Extract materials
    if (curr_aimesh->mMaterialIndex > 0) {
      aiMaterial* mat = scene->mMaterials[curr_aimesh->mMaterialIndex];
      load_material(mesh, mat, aiTextureType_DIFFUSE);
      load_material(mesh, mat, aiTextureType_SPECULAR);
    }

    // Extract name
    mesh.name = std::string{curr_aimesh->mName.C_Str()};

    meshes.emplace_back(std::move(mesh));
  }
}

inline model::model(std::vector<mesh_data> meshes) {
  for (auto& data : meshes) {
    std::unordered_map<material_type, texture2d> materials;
    for (auto& [type, tex_data] : data.materials) {
      materials.emplace(std::make_pair(type, 
        impl::load_texture<1u>(tex_data.pixels, tex_data.dim, tex_data.format, tex_data.filter, tex_data.wrap)
      ));
    }

    mesh mesh {
      mesh_primitive::triangles,
      &data.vertices[0], data.vertices.size()*sizeof(mesh_data::vertex), mesh_buffer::static_draw,
      &data.indices[0], data.indices.size()*sizeof(uint), mesh_buffer::static_draw,
      shadatt_coords3d{}, shadatt_normals3d{}, shadatt_texcoords3d{}
    };

    _meshes.emplace(std::make_pair(
      std::move(data.name),
      textured_mesh{std::move(mesh), std::move(materials)}
    ));
  }
}


inline model load_model(std::string_view path, tex_filter mat_filter, tex_wrap mat_wrap) {
  model_data::loader loader;
  return loader(model_data{path, mat_filter, mat_wrap});
}

} // namespace ntf
