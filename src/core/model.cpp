#include "core/model.hpp"
#include "core/logger.hpp"

namespace ntf::shogle {

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
  logger::verbose("[Model::Mesh] Initialized mesh buffers (id: {})", this->vao);

  for (const auto& tex_data : mesh.tex) {
    // Do not pass tex_data as an unique_ptr
    this->tex.emplace_back(Texture{&tex_data});
  }
  logger::verbose("[Model::Mesh] Created mesh textures (id: {})", this->vao);

  logger::verbose("[Model::Mesh] Created mesh (id: {})", this->vao);
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
  logger::verbose("[Model::Mesh] Deleted mesh (id: {})", id);
}

Model::Model(std::unique_ptr<ModelData> data) {
  for (const auto& mesh_data : data->meshes) {
    this->meshes.emplace_back(Model::Mesh{mesh_data});
  }
  logger::debug("[Model] Created model");
}

} // namespace ntf::shogle
