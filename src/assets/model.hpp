#pragma once

#include "./assets.hpp"
#include "./texture.hpp"
#include "../render/render.hpp"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace ntf {

using material_index = uint32_t;

template<typename Texture>
class material {
public:
  using texture_type = Texture;

  using iterator = std::vector<texture_type>::iterator;
  using const_iterator = std::vector<texture_type>::const_iterator;

public:
  material() = default;
  material(std::vector<texture_type> materials, 
           std::unordered_map<material_category, material_index> material_types);

public:
  const texture_type& operator[](material_index pos) const;
  texture_type& operator[](material_index pos);

  std::optional<material_index> find(material_category type);

public:
  std::size_t size() const { return _materials.size();}

  iterator begin() { return _materials.begin(); }
  const_iterator begin() const { return _materials.begin(); }
  const_iterator cbegin() const { return _materials.cbegin(); }

  iterator end() { return _materials.end(); }
  const_iterator end() const { return _materials.end(); }
  const_iterator cend() const { return _materials.cend(); }

private:
  std::vector<texture_type> _materials;
  std::unordered_map<material_category, std::size_t> _material_types;
};

template<typename Mesh, typename Texture>
class model {
public:
  using mesh_type = Mesh;
  using texture_type = Texture;

public:
  model() = default;

  model(std::vector<mesh_type> meshes, std::unordered_map<std::string, std::size_t> names,
        std::vector<material<texture_type>> materials);

private:
  std::vector<mesh_type> _meshes;
  std::unordered_map<std::string, std::size_t> _mesh_names;
  std::vector<material<texture_type>> _materials;
};


struct basic_vertex {
  using att0 = shader_attribute<0, vec3>;
  using att1 = shader_attribute<1, vec3>;
  using att2 = shader_attribute<2, vec2>;

  vec3 coord;
  vec3 normal;
  vec2 tex_coord;
};


template<typename Mesh, typename Texture, typename Vertex = basic_vertex>
class assimp_data_loader {
public:
  using resource_type = model<Mesh, Texture>;

private:
  using texture_type = Texture;
  using vertex_type = Vertex;

  struct basic_mesh_data {
    std::string name;
    std::vector<vertex_type> vertices;
    std::vector<uint> indices;
    std::vector<std::pair<material_category, basic_texture_data<texture_type>>> materials;
  };

public:
  using data_type = std::vector<basic_mesh_data>;

public:
  bool resource_load(std::string_view path);
  void resource_unload(bool overwrite);

  std::optional<resource_type> make_resource() const;

private:
  data_type _data;
};

template<typename Texture>
struct mesh_data {
  using texture_data_type = texture_data<Texture>;

  struct vertex {
    using att_coord    = shader_attribute<0, vec3>;
    using att_normal   = shader_attribute<1, vec3>;
    using att_texcoord = shader_attribute<2, vec2>;

    vec3 coord;
    vec3 normal;
    vec2 tex_coord;
  };

  std::string name;
  std::vector<vertex> vertices;
  std::vector<uint> indices;
  std::vector<std::pair<material_category, texture_data_type>> materials;
};


template<typename Mesh, typename Texture>
class model {
public:
  using texture_type = Texture;
  using mesh_type = Mesh;

  using mesh_data_type = mesh_data<Texture>;
  struct data_type;

  class textured_mesh;

public:
  model() = default;
  model(std::vector<mesh_data_type> meshes);

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


template<typename Mesh, typename Texture>
struct model<Mesh, Texture>::data_type {
public:
  using texture_data_type = texture_data<Texture>;
  using mesh_data_type = mesh_data<Texture>;

public:
  struct loader {
    model operator()(data_type data) {
      return model{std::move(data.meshes)};
    }
    model operator()(std::string path, tex_filter filter, tex_wrap wrap) {
      return (*this)(data_type{path, filter, wrap});
    }
  };

public:
  data_type(std::string_view path_, tex_filter filter_, tex_wrap wrap_);

public:
  std::vector<mesh_data_type> meshes;
};


template<typename Mesh, typename Texture>
class model<Mesh, Texture>::textured_mesh {
public:
  using texture_type = Texture;
  using mesh_type = Mesh;

public:
  textured_mesh(mesh_type mesh, std::unordered_map<material_category, texture_type> materials) :
    _mesh(std::move(mesh)), _materials(std::move(materials)) {}

public:
  const mesh_type& mesh() const { return _mesh; }
  const texture_type& operator[](material_category material) const { return _materials.at(material); }

  auto cbegin() const { return _materials.cbegin(); }
  auto cend() const { return _materials.cend(); }
  auto begin() { return _materials.begin(); }
  auto end() { return _materials.end(); }

  size_t size() const { return _materials.size(); }

private:
  mesh_type _mesh;
  std::unordered_map<material_category, texture_type> _materials;
};

template<typename Mesh, typename Texture>
model<Mesh, Texture>::data_type::data_type(std::string_view path_, tex_filter filter_,
                                           tex_wrap wrap_) {
  Assimp::Importer import;
  const aiScene* scene = import.ReadFile(path_.data(), aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    throw ntf::error{"[ntf::model_data] ASSIMP error: {}", import.GetErrorString()};
  }

  auto dir = file_dir(path_.data());
  std::vector<std::string> loaded_materials;

  auto load_material = [&](mesh_data_type& mesh, aiMaterial* aimat, aiTextureType aitype) {
    material_category mat_type;
    switch (aitype) {
      case aiTextureType_SPECULAR: {
        mat_type = material_category::specular;
        break;
      }
      default: {
        mat_type = material_category::diffuse;
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
          mat_type, texture_data_type{tex_path, filter_, wrap_}
        ));
        loaded_materials.emplace_back(std::move(tex_path));
      }
    }
  };

  for (size_t i = 0; i < scene->mNumMeshes; ++i) {
    mesh_data_type mesh;
    aiMesh* curr_aimesh = scene->mMeshes[i];

    // Extract vertices
    for (size_t j = 0; j < curr_aimesh->mNumVertices; ++j) { 
      typename mesh_data_type::vertex vert;
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

template<typename Mesh, typename Texture>
model<Mesh, Texture>::model(std::vector<mesh_data_type> meshes) {
  for (auto& data : meshes) {
    std::unordered_map<material_category, texture_type> materials;
    for (auto& [type, tex_data] : data.materials) {
      typename texture_type::loader tex_loader;
      materials.emplace(std::make_pair(type, 
        tex_loader(tex_data.pixels, tex_data.dim, tex_data.format, tex_data.filter, tex_data.wrap)
      ));
    }

    mesh_type mesh {
      mesh_primitive::triangles,
      &data.vertices[0], data.vertices.size()*sizeof(mesh_data_type::vertex),
      mesh_buffer::static_draw,
      &data.indices[0], data.indices.size()*sizeof(uint),
      mesh_buffer::static_draw,
      typename mesh_data_type::att_coords{}, typename mesh_data_type::att_normal{},
      typename mesh_data_type::att_texcoord{}
    };

    _meshes.emplace(std::make_pair(
      std::move(data.name),
      textured_mesh{std::move(mesh), std::move(materials)}
    ));
  }
}

} // namespace ntf
