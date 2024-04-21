#include "res/model.hpp"

#include "core/log.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace ntf {

// Model::data_t
void _load_materials(std::vector<Material::data_t>& material_datas, aiMaterial* mat, aiTextureType type, const std::string& dir) {
  for (size_t i = 0; i < mat->GetTextureCount(type); ++i) {
    aiString filename;
    mat->GetTexture(type, i, &filename);
    std::string tex_path = dir+"/"+std::string{filename.C_Str()};

    bool skip = false;
    for (const auto& material : material_datas) { 
      if (std::strcmp(material.path.data(), tex_path.data()) == 0) {
        skip = true;
        break;
      }
    }
    if (!skip) {
      material_datas.emplace_back(MaterialData::args_t {
        .path = tex_path,
        .type = type
      });
      Log::verbose("[ModelData] Mesh material extracted (path: {})", tex_path);
    }
  }
}

ModelData::ModelData(std::string path) {
  Assimp::Importer import;
  const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    Log::error("[ModelData] ASSIMP: {}", import.GetErrorString());
  }

  for (size_t i = 0; i < scene->mNumMeshes; ++i) {
    MeshData mesh{};
    aiMesh* curr_aimesh = scene->mMeshes[i];

    // Extract vertices
    for (size_t j = 0; j < curr_aimesh->mNumVertices; ++j) {
      Vertex vert{};
      vert.ver_coord = vec3{
        curr_aimesh->mVertices[j].x,
        curr_aimesh->mVertices[j].y,
        curr_aimesh->mVertices[j].z
      };
      vert.ver_norm = vec3{
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

      mesh.vert.push_back(std::move(vert));
    }

    // Extract indices
    for (size_t j = 0; j < curr_aimesh->mNumFaces; ++j) {
      aiFace face = curr_aimesh->mFaces[j];
      for (size_t k = 0; k < face.mNumIndices; ++k) {
        mesh.ind.push_back(face.mIndices[k]);
      }
    }

    // Extract materials
    if (curr_aimesh->mMaterialIndex > 0) {
      aiMaterial* mat = scene->mMaterials[curr_aimesh->mMaterialIndex];
      std::string dir = path.substr(0, path.find_last_of('/')); // Get model directory
      _load_materials(mesh.materials, mat, aiTextureType_DIFFUSE, dir);
      _load_materials(mesh.materials, mat, aiTextureType_SPECULAR, dir);
    }

    meshes.push_back(std::move(mesh));
  }
  Log::verbose("[ModelData] Model data extracted (path: {})", path);
}

// Model
ModelRes::Mesh::Mesh(const ModelData::MeshData& mesh) {
  using Vertex = ModelData::Vertex;
  this->_indices = mesh.ind.size();

  glGenVertexArrays(1, &this->_vao);
  glGenBuffers(1, &this->_vbo);
  glGenBuffers(1, &this->_ebo);

  glBindVertexArray(this->_vao);
  glBindBuffer(GL_ARRAY_BUFFER, this->_vbo);
  glBufferData(GL_ARRAY_BUFFER, mesh.vert.size()*sizeof(Vertex), &mesh.vert[0], GL_STATIC_DRAW);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.vert.size()*sizeof(GLuint), &mesh.ind[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, ver_norm));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coord));

  glBindVertexArray(0);

  for (const auto& material_data : mesh.materials) {
    // Do not pass tex_data as an unique_ptr
    this->materials.emplace_back(Material{&material_data});
  }

  Log::verbose("[Model] Mesh created (vao-id: {})", this->_vao);
}

ModelRes::Mesh::Mesh(Mesh&& m) noexcept :
  materials(std::move(m.materials)),
  _vao(std::move(m._vao)),
  _vbo(std::move(m._vbo)),
  _ebo(std::move(m._ebo)),
  _indices(std::move(m._indices)) {

  m._vao = 0;
  m._vbo = 0;
  m._ebo = 0;
}

ModelRes::Mesh& ModelRes::Mesh::operator=(Mesh&& m) noexcept {
  this->materials = std::move(m.materials);
  this->_vao = std::move(m._vao);
  this->_vbo = std::move(m._vbo);
  this->_ebo = std::move(m._ebo);
  this->_indices = std::move(m._indices);

  m._vao = 0;
  m._vbo = 0;
  m._ebo = 0;

  return *this;
}

ModelRes::Mesh::~Mesh() {
  if (this->_vao == 0) return;
  GLint id = this->_vao;
  glBindVertexArray(this->_vao);
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &this->_vao);
  glDeleteBuffers(1, &this->_ebo);
  glDeleteBuffers(1, &this->_vbo);
  Log::verbose("[Model] Mesh deleted (vao-id: {})", id);
}

ModelRes::ModelRes(const ModelRes::data_t* data) {
  for (const auto& mesh_data : data->meshes) {
    this->meshes.emplace_back(ModelRes::Mesh{mesh_data});
  }
}

} // namespace ntf
