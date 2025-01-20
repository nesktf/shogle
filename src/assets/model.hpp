#pragma once

#include "./assets.hpp"
#include "./texture.hpp"
#include "./meshes.hpp"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace ntf {

enum class r_material_type {
  diffuse = 0,
  specular,
};

template<typename Vertex>
struct assimp_mesh_loader;

template<>
struct assimp_mesh_loader<pnt_vertex> {
  pnt_vertex operator()(const aiMesh& mesh, uint32 index) {
    pnt_vertex vert;
    auto& ai_pos = mesh.mVertices[index];
    auto& ai_norm = mesh.mNormals[index];

    vert.position.x = ai_pos.x;
    vert.position.y = ai_pos.y;
    vert.position.z = ai_pos.z;

    vert.normal.x = ai_norm.x;
    vert.normal.y = ai_norm.y;
    vert.normal.z = ai_norm.z;

    if (mesh.mTextureCoords[0]) {
      auto& ai_uv = mesh.mTextureCoords[0][index];
      vert.uv.x = ai_uv.x;
      vert.uv.y = ai_uv.y;
    } else {
      vert.uv.x = 0;
      vert.uv.y = 0;
    }

    return vert;
  };
};

template<>
struct assimp_mesh_loader<pn_vertex> {
  pn_vertex operator()(const aiMesh& mesh, uint32 index) {
    pn_vertex vert;
    auto& ai_pos = mesh.mVertices[index];
    auto& ai_norm = mesh.mNormals[index];

    vert.position.x = ai_pos.x;
    vert.position.y = ai_pos.y;
    vert.position.z = ai_pos.z;

    vert.normal.x = ai_norm.x;
    vert.normal.y = ai_norm.y;
    vert.normal.z = ai_norm.z;

    return vert;
  };
};

template<typename Vertex, typename Alloc = std::allocator<uint8>>
class model_data {
public:
  using allocator_type = Alloc; // TODO: Actually use the allocator
  using texture_data_type = texture_data<Alloc>;
  using vertex_type = Vertex;

  struct material_data {
    r_material_type type;
    texture_data_type texture;
  };

  struct mesh_data {
    std::vector<vertex_type> vertices;
    std::vector<uint32> indices;
    std::vector<material_data> materials;

    size_t vertices_size() const { return vertices.size()*sizeof(vertex_type); }
    size_t indices_size() const { return indices.size()*sizeof(uint32); }
  };

private:
  using mesh_loader = assimp_mesh_loader<vertex_type>;

public:
  model_data()
  noexcept(std::is_nothrow_default_constructible_v<Alloc>) :
    _alloc{Alloc{}} {}

  explicit model_data(const Alloc& alloc)
  noexcept(std::is_nothrow_copy_constructible_v<Alloc>) :
    _alloc{alloc} {}

  explicit model_data(std::string_view path) noexcept { load(path); }

  model_data(std::string_view path, const Alloc& alloc)
  noexcept(std::is_nothrow_copy_constructible_v<Alloc>) :
    _alloc{alloc} { load(path); }

public:
  void load(std::string_view path) noexcept {
    NTF_ASSERT(!has_data());
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path.data(), aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
      SHOGLE_LOG(error, "[ntf::model_data] ASSIMP error: {}", import.GetErrorString());
      return;
    }

    auto dir = file_dir(path);
    if (!dir) {
      SHOGLE_LOG(error, "[ntf::model_data] Invalid file path: \"{}\"", path);
      return;
    }
    std::vector<std::string> loaded_materials;

    auto load_material = [&](mesh_data& mesh, aiMaterial* mat, aiTextureType type) {
      NTF_ASSERT(mat);

      for (uint32 i = 0; i < mat->GetTextureCount(type); ++i) {
        aiString filename;
        mat->GetTexture(type, i, &filename);
        auto tex_path = *dir + "/" + std::string{filename.C_Str()};
        bool skip = false;

        for (const auto& mat_path : loaded_materials) {
          if (std::strcmp(mat_path.data(), tex_path.data()) == 0) {
            skip = true;
            break;
          }
        }
        if (skip) {
          continue;
        }

        texture_data_type tex_data;
        tex_data.load(tex_path);
        if (!tex_data) {
          SHOGLE_LOG(error, "[ntf::model_data] Failed to load texture: \"{}\"", tex_path);
          continue;
        }
        mesh.materials.emplace_back(assimp_material_cast(type), std::move(tex_data));
        loaded_materials.emplace_back(std::move(tex_path));
      }
    };

    _meshes.reserve(scene->mNumMeshes);
    SHOGLE_LOG(verbose, "[ntf::model_data] Found {} mesh(es) for model \"{}\"",
               scene->mNumMeshes, path);
    for (uint32 i = 0; i < scene->mNumMeshes; ++i) {
      mesh_data mesh;
      aiMesh* ai_mesh = scene->mMeshes[i];
      NTF_ASSERT(ai_mesh);

      // Vertices
      mesh.vertices.reserve(ai_mesh->mNumVertices);
      for (uint32 j = 0; j < ai_mesh->mNumVertices; ++j) {
        mesh.vertices.emplace_back(mesh_loader{}(*ai_mesh, j));
      }

      // Indices
      // mesh.indices.reserve(ai_mesh->mNumFaces);
      for (uint32 j = 0; j < ai_mesh->mNumFaces; ++j) {
        auto& face = ai_mesh->mFaces[j];
        for (uint32 k = 0; k < face.mNumIndices; ++k) {
          mesh.indices.emplace_back(face.mIndices[k]);
        }
      }

      if (ai_mesh->mMaterialIndex > 0) {
        aiMaterial* mat = scene->mMaterials[ai_mesh->mMaterialIndex];
        load_material(mesh, mat, aiTextureType_DIFFUSE);
        load_material(mesh, mat, aiTextureType_SPECULAR);
      }


      auto [_, emplaced] = _mesh_names.try_emplace(ai_mesh->mName.C_Str(), i);
      NTF_ASSERT(emplaced);
      _meshes.emplace_back(std::move(mesh));
    }
  }

  void unload() noexcept {
    NTF_ASSERT(has_data());
    _meshes.clear();
    _mesh_names.clear();
  }

public:
  [[nodiscard]] static inline r_material_type assimp_material_cast(aiTextureType type) {
    switch (type) {
      case aiTextureType_SPECULAR: return r_material_type::specular;
      case aiTextureType_DIFFUSE:  return r_material_type::diffuse;

      default: break;
    }

    // TODO: Handle more material types :p
    NTF_UNREACHABLE();
  }

public:
  const std::vector<mesh_data>& data() const { return _meshes; }
  std::vector<mesh_data>& data() { return _meshes; }
  uint32 mesh_count() const { return _meshes.size(); }

  const std::unordered_map<std::string, uint32>& names() const { return _mesh_names; }
  std::unordered_map<std::string, uint32>& names() { return _mesh_names; }

  bool has_data() const { return mesh_count() > 0; }
  explicit operator bool() const { return has_data(); }

private:
  [[maybe_unused]] Alloc _alloc;
  std::vector<mesh_data> _meshes;
  std::unordered_map<std::string, uint32> _mesh_names;
};

} // namespace ntf
