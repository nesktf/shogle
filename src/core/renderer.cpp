#include "core/renderer.hpp"
#include "core/model_object.hpp"
#include "core/sprite_object.hpp"
#include "core/logger.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace ntf::shogle {

Renderer::Renderer() {
  // Create sprite renderer (quad)
  float quad_vert[] = {
    // coord      // tex_coord
    -0.5f, -0.5f, 0.0f, 0.0f,
     0.5f, -0.5f, 1.0f, 0.0f,
     0.5f,  0.5f, 1.0f, 1.0f,
    -0.5f,  0.5f, 0.0f, 1.0f
  };
  GLuint quad_ind[] = {
    0, 1, 2,
    0, 2, 3
  };

  glGenVertexArrays(1, &this->q_VAO);
  glGenBuffers(1, &this->q_VBO);
  glGenBuffers(1, &this->q_EBO);

  glBindVertexArray(this->q_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, this->q_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vert), quad_vert, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->q_EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_ind), quad_ind, GL_STATIC_DRAW);

  // Sprite shader needs a vec4f with 2d coords and tex coords
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
  glBindVertexArray(0);
  logger::verbose("[Renderer] Created sprite VAO (id: {})", this->q_VAO);

  this->pproj_m   = glm::mat4(1.0f);
  this->oproj_m   = glm::mat4(1.0f);
  this->view_pos  = glm::vec3(0.0f);
  this->dir_vec   = glm::vec3{0.0f, 0.0f, -1.0f};
  this->up_vec    = glm::vec3{0.0f, 1.0f, 0.0f};
  
  logger::debug("[Renderer] Initialized");
}

Renderer::~Renderer() {
  GLint vao = this->q_VAO;
  glBindVertexArray(this->q_VAO);
  glDisableVertexAttribArray(0);
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &this->q_VAO);
  glDeleteBuffers(1, &this->q_EBO);
  glDeleteBuffers(1, &this->q_VBO);
  logger::verbose("[Renderer] Deleted sprite VAO (id: {})", vao);

  logger::debug("[Renderer] Terminated");
}

void Renderer::draw(Shader& shader, const ModelObject& obj) const {
  shader.set_mat4("proj", pproj_m);
  shader.set_mat4("view", view_m);
  shader.set_vec3("view_pos", view_pos);
  shader.set_mat4("model", obj.model_m);

  for (const auto& mesh : obj.model.get().meshes) {
    size_t diff_n = 1;
    size_t spec_n = 1;
    for (size_t i = 0; i < mesh.tex.size(); ++i) {
      const auto& tex = mesh.tex[i];
      tex.bind_material(shader, tex.type() == aiTextureType_DIFFUSE ? diff_n++ : spec_n++, i);
    }

    glBindVertexArray(mesh.id());
    glDrawElements(GL_TRIANGLES, mesh.ind(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }
}

void Renderer::draw(Shader& shader, const SpriteObject& obj) const {
  shader.set_mat4("proj", oproj_m);
  shader.set_mat4("model", obj.model_m);
  
  glBindTexture(GL_TEXTURE_2D, obj.texture.get().id());
  glActiveTexture(GL_TEXTURE0);
  shader.set_int("sprite_texture", 0);

  glBindVertexArray(q_VAO);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void Renderer::update_proj_m(size_t width, size_t height) {
  this->pproj_m = glm::perspective(glm::radians(this->fov), (float)width/(float)height, 0.1f, 100.0f);
  this->oproj_m = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
}

void Renderer::update_view_m(float yaw, float pitch) {
  this->dir_vec = glm::normalize(glm::vec3{
    glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch)),
    glm::sin(glm::radians(pitch)),
    glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch))
  });

  this->view_m = glm::lookAt(
    this->view_pos,
    this->view_pos + this->dir_vec,
    this->up_vec
  );
}

void Renderer::update_view_m(const glm::vec3& vec) {
  this->dir_vec = vec; // Assume normalized input

  this->view_m = glm::lookAt(
    this->view_pos,
    this->view_pos + this->dir_vec,
    this->up_vec
  );
}

}
