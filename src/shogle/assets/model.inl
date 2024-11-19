#define SHOGLE_ASSETS_MODEL_INL
#include "./model.hpp"
#undef SHOGLE_ASSETS_MODEL_INL

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace ntf {

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
