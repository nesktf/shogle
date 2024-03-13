#include "resources/model_data.hpp"
#include "core/logger.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace ntf::shogle {

template<typename T>
void _load_materials(T& textures, aiMaterial* mat, aiTextureType type, const std::string& dir) {
  for (size_t i = 0; i < mat->GetTextureCount(type); ++i) {
    aiString filename;
    mat->GetTexture(type, i, &filename);
    std::string tex_path = dir+"/"+std::string{filename.C_Str()};
    logger::debug("[ModelData] Tex path: {}", tex_path);

    bool skip = false;
    for (const auto& tex : textures) {
      if (std::strcmp(tex.path.data(), tex_path.data()) == 0) {
        skip = true;
        break;
      }
    }
    if (!skip) {
      textures.emplace_back(tex_path.c_str(), GL_TEXTURE_2D, type, TextureData::Type::ModelTex);
    }
  }
}

ModelData::ModelData(const char* path) {
  Assimp::Importer import;
  const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    logger::error("[ModelData] ASSIMP: {}", import.GetErrorString());
  }

  for (size_t i = 0; i < scene->mNumMeshes; ++i) {
    MeshData mesh{};
    aiMesh* curr_aimesh = scene->mMeshes[i];

    // Extract vertices
    for (size_t j = 0; j < curr_aimesh->mNumVertices; ++j) {
      Vertex vert{};
      vert.ver_coord = glm::vec3{
        curr_aimesh->mVertices[j].x,
        curr_aimesh->mVertices[j].y,
        curr_aimesh->mVertices[j].z
      };
      vert.ver_norm = glm::vec3{
        curr_aimesh->mNormals[j].x,
        curr_aimesh->mNormals[j].y,
        curr_aimesh->mNormals[j].z
      };

      if (curr_aimesh->mTextureCoords[0]) {
        vert.tex_coord = glm::vec2{
          curr_aimesh->mTextureCoords[0][j].x,
          curr_aimesh->mTextureCoords[0][j].y
        };
      }

      mesh.vert.push_back(std::move(vert));
    }

    // Extract indices
    for (size_t j = 0; j < curr_aimesh->mNumFaces; ++j) {
      aiFace face = curr_aimesh->mFaces[j];
      for (size_t k = 0; k < face.mNumIndices; ++k) {
        mesh.ind.push_back(face.mIndices[j]);
      }
    }

    // Extract materials
    if (curr_aimesh->mMaterialIndex >= 0) {
      aiMaterial* mat = scene->mMaterials[curr_aimesh->mMaterialIndex];
      std::string f_path {path};
      std::string dir = f_path.substr(0, f_path.find_last_of('/')); // Get model directory
      _load_materials(mesh.tex, mat, aiTextureType_DIFFUSE, dir);
      _load_materials(mesh.tex, mat, aiTextureType_SPECULAR, dir);
    }

    meshes.push_back(std::move(mesh));
  }
}

} // namespace ntf::shogle
