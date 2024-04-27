#include <shogle/render/res/model.hpp>

#include <shogle/core/log.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace ntf::render {

Material::Material(const Material::data_t* data) :
  Texture(&data->tex_data) {

  switch (data->type) {
    case aiTextureType_DIFFUSE: {
      _uniform_basename = "material.diffuse";
      _type = MaterialType::Diffuse;
      break;
    }
    case aiTextureType_SPECULAR: {
      _uniform_basename = "material.specular";
      _type = MaterialType::Specular;
      break;
    }
    default: {
      _uniform_basename = "material.diffuse";
      _type = MaterialType::Diffuse;
      break;
    }
  }
}

void Material::bind_uniform(const Shader* shader, size_t tex_num, size_t tex_ind) const {
  std::string uniform_name {_uniform_basename + std::to_string(tex_num) };
  glBindTexture(_dim, _tex);
  glActiveTexture(GL_TEXTURE0 + tex_ind);
  shader->unif_int(uniform_name.c_str(), tex_ind);
  shader->unif_float("material.col_shiny", 1.0f); // TODO: don't use fixed value
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

} // namespace ntf::render
