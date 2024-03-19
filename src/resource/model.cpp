#include "resource/model.hpp"

#include "log.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace ntf::shogle::res {

// Model::data_t
template<typename T>
void _load_materials(T& textures, aiMaterial* mat, aiTextureType type, const std::string& dir) {
  for (size_t i = 0; i < mat->GetTextureCount(type); ++i) {
    aiString filename;
    mat->GetTexture(type, i, &filename);
    std::string tex_path = dir+"/"+std::string{filename.C_Str()};
    log::debug("[ModelData] Tex path: {}", tex_path);

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
    log::error("[ModelData] ASSIMP: {}", import.GetErrorString());
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

// Model
Model::Mesh::Mesh(const ModelData::MeshData& mesh) {
  using Vertex = ModelData::Vertex;
  this->indices = mesh.ind.size();

  glGenVertexArrays(1, &this->vao);
  glGenBuffers(1, &this->vbo);
  glGenBuffers(1, &this->ebo);

  glBindVertexArray(this->vao);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferData(GL_ARRAY_BUFFER, mesh.vert.size()*sizeof(Vertex), &mesh.vert[0], GL_STATIC_DRAW);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.vert.size()*sizeof(GLuint), &mesh.ind[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, ver_norm));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coord));

  glBindVertexArray(0);
  log::verbose("[Model::Mesh] Initialized mesh buffers (id: {})", this->vao);

  for (const auto& tex_data : mesh.tex) {
    // Do not pass tex_data as an unique_ptr
    this->tex.emplace_back(Texture{&tex_data});
  }
  log::verbose("[Model::Mesh] Created mesh textures (id: {})", this->vao);

  log::verbose("[Model::Mesh] Created mesh (id: {})", this->vao);
}

Model::Mesh::~Mesh() {
  GLint id = this->vao;
  glBindVertexArray(this->vao);
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &this->vao);
  glDeleteBuffers(1, &this->ebo);
  glDeleteBuffers(1, &this->vbo);
  log::verbose("[Model::Mesh] Deleted mesh (id: {})", id);
}

Model::Model(const Model::data_t* data) {
  for (const auto& mesh_data : data->meshes) {
    this->meshes.emplace_back(Model::Mesh{mesh_data});
  }
  log::debug("[Model] Created model");
}

} // namespace ntf::shogle::res

