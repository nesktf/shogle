#pragma once

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

template<typename T, typename Vertex>
concept is_vertex_loader = is_complete<T> && requires(T obj, const aiMesh& mesh, uint32 index) {
  { obj(mesh, index) } -> std::convertible_to<Vertex>;
};
// It should check for undefined assimp_mesh_loader
// static_assert(!is_vertex_loader<assimp_mesh_loader<void>, void>);

template<>
struct assimp_mesh_loader<pnt_vertex> {
  constexpr pnt_vertex operator()(const aiMesh& mesh, uint32 index) {
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
static_assert(is_vertex_loader<assimp_mesh_loader<pnt_vertex>, pnt_vertex>);

template<>
struct assimp_mesh_loader<pn_vertex> {
  constexpr pn_vertex operator()(const aiMesh& mesh, uint32 index) {
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
static_assert(is_vertex_loader<assimp_mesh_loader<pn_vertex>, pn_vertex>);

template<typename Vertex, typename Alloc = std::allocator<Vertex>>
class model_data {
public:
  using allocator_type = Alloc;
  using vertex_type = Vertex;

  struct material_data {
    template<typename MatAlloc>
    material_data(const MatAlloc& alloc) :
      texture(alloc) {}

    r_material_type type;
    texture_data<rebind_alloc_t<uint8, Alloc>> texture;
  };

  struct mesh_data {
    template<typename MeshAlloc>
    mesh_data(const MeshAlloc& alloc) :
      name(alloc), vertices(alloc), indices(alloc), materials(alloc) {}

    std::basic_string<char, std::char_traits<char>, rebind_alloc_t<char, Alloc>> name;
    std::vector<vertex_type, Alloc> vertices;
    std::vector<uint32, rebind_alloc_t<uint32, Alloc>> indices;
    std::vector<material_data, rebind_alloc_t<material_data, Alloc>> materials;

    [[nodiscard]] size_t vertices_size() const { return vertices.size()*sizeof(vertex_type); }
    [[nodiscard]] size_t indices_size() const { return indices.size()*sizeof(uint32); }
  };

  using mesh_vec = std::vector<mesh_data, rebind_alloc_t<mesh_data, Alloc>>;

  using value_type = vertex_type;
  using iterator = mesh_vec::iterator;
  using const_iterator = mesh_vec::const_iterator;

public:
  model_data()
  noexcept(std::is_nothrow_default_constructible_v<Alloc>) :
    _meshes(Alloc{}) {}

  explicit model_data(const Alloc& alloc)
  noexcept(std::is_nothrow_copy_constructible_v<Alloc>) :
    _meshes(alloc) {}

  explicit model_data(std::string_view path)
  noexcept(std::is_nothrow_default_constructible_v<Alloc>) :
    _meshes(Alloc{}) { load(path); }

  template<is_vertex_loader<Vertex> MeshLoader>
  model_data(std::string_view path, MeshLoader&& mesh_loader)
  noexcept(std::is_nothrow_default_constructible_v<Alloc>) : 
    _meshes(Alloc{}) { load(path, std::forward<MeshLoader>(mesh_loader)); }

  explicit model_data(std::string_view path, const Alloc& alloc)
  noexcept(std::is_nothrow_copy_constructible_v<Alloc>) :
    _meshes(alloc) { load(path); }

  template<is_vertex_loader<Vertex> MeshLoader>
  model_data(std::string_view path, const Alloc& alloc, MeshLoader&& mesh_loader)
  noexcept(std::is_nothrow_copy_constructible_v<Alloc>) :
    _meshes(alloc) { load(path, std::forward<MeshLoader>(mesh_loader)); }

public:
  template<is_vertex_loader<Vertex> MeshLoader = assimp_mesh_loader<Vertex>>
  void load(std::string_view path, MeshLoader&& mesh_loader = {}) noexcept {
    NTF_ASSERT(!has_data());
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path.data(), aiProcess_Triangulate);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
      SHOGLE_LOG(error, "[ntf::model_data] ASSIMP error: {}", import.GetErrorString());
      return;
    }

    auto dir = file_dir(path);
    if (!dir) {
      SHOGLE_LOG(error, "[ntf::model_data] Invalid file path: \"{}\"", path);
      return;
    }
    std::vector<std::string> loaded_materials; // Maybe use the allocator here?

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

        decltype(material_data::texture) tex_data{_meshes.get_allocator()};
        tex_data.load(tex_path);
        if (!tex_data) {
          SHOGLE_LOG(error, "[ntf::model_data] Failed to load texture: \"{}\"", tex_path);
          continue;
        }
        mesh.materials.emplace_back(_meshes.get_allocator());
        auto& mat = mesh.materials.back();
        mat.type = assimp_material_cast(type);
        mat.texture = std::move(tex_data);
        SHOGLE_LOG(verbose, "[ntf::model_data] Found texture \"{}\" for model \"{}\"",
                   tex_path, path);
        loaded_materials.emplace_back(std::move(tex_path));
      }
    };

    _meshes.reserve(scene->mNumMeshes);
    SHOGLE_LOG(verbose, "[ntf::model_data] Found {} mesh(es) for model \"{}\"",
               scene->mNumMeshes, path);
    for (uint32 i = 0; i < scene->mNumMeshes; ++i) {
      mesh_data mesh{_meshes.get_allocator()};
      aiMesh* ai_mesh = scene->mMeshes[i];
      NTF_ASSERT(ai_mesh);

      // Vertices
      mesh.vertices.reserve(ai_mesh->mNumVertices);
      for (uint32 j = 0; j < ai_mesh->mNumVertices; ++j) {
        mesh.vertices.emplace_back(mesh_loader(*ai_mesh, j));
      }

      // Indices
      // mesh.indices.reserve(ai_mesh->mNumFaces);
      for (uint32 j = 0; j < ai_mesh->mNumFaces; ++j) {
        auto& face = ai_mesh->mFaces[j];
        for (uint32 k = 0; k < face.mNumIndices; ++k) {
          mesh.indices.emplace_back(face.mIndices[k]);
        }
      }

      // Materials
      if (ai_mesh->mMaterialIndex > 0) {
        aiMaterial* mat = scene->mMaterials[ai_mesh->mMaterialIndex];
        load_material(mesh, mat, aiTextureType_DIFFUSE);
        load_material(mesh, mat, aiTextureType_SPECULAR);
      }

      mesh.name.resize(ai_mesh->mName.length);
      std::memcpy(mesh.name.data(), ai_mesh->mName.C_Str(), ai_mesh->mName.length);
      SHOGLE_LOG(verbose, "[ntf::model_data] Loaded mesh \"{}\" for \"{}\"", mesh.name, path);
      _meshes.emplace_back(std::move(mesh));
    }
  }

  void unload() noexcept {
    NTF_ASSERT(has_data());
    _meshes.clear();
  }

public:
  const mesh_vec& meshes() const { return _meshes; }
  // mesh_vec& meshes() { return _meshes; }
  [[nodiscard]] uint32 mesh_count() const { return _meshes.size(); }

  [[nodiscard]] bool has_data() const { return mesh_count() > 0; }
  explicit operator bool() const { return has_data(); }

  // iterator begin() { return _meshes.begin(); }
  const_iterator begin() const { return _meshes.begin(); }
  const_iterator cbegin() const { return _meshes.cbegin(); }

  // iterator end() { return _meshes.end(); }
  const_iterator end() const { return _meshes.end(); }
  const_iterator cend() const { return _meshes.cend(); }

  // mesh_data& operator[](uint32 index) {
  //   NTF_ASSERT(index < _meshes.size());
  //   return _meshes[index];
  // }
  const mesh_data& operator[](uint32 index) const {
    NTF_ASSERT(index < _meshes.size());
    return _meshes[index];
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

private:
  mesh_vec _meshes;
};

} // namespace ntf
