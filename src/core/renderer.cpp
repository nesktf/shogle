#include "core/renderer.hpp"
#include "core/model.hpp"
#include "core/sprite.hpp"

namespace ntf::shogle {

Renderer::Renderer() {
  // Create sprite renderer (quad)
  float quad_vert[] = {
    -0.5f, -0.5f, 0.0f, 0.0f,
    0.5f, -0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, 0.0f, 1.0f
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

  // Sprites need a vec4f with 2d coords and tex coords
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
  glBindVertexArray(0);

}

template<>
void Renderer::draw(Shader& shader, Model& obj) {
  shader.set_mat4("proj", pproj_m);
  shader.set_mat4("view", view_m);
  shader.set_vec3("view_pos", view_pos);
  shader.set_mat4("model", obj.model_m);

  for (auto& vao : obj.vaos) {
    size_t diff_n = 1;
    size_t spec_n = 1;
    for (size_t i = 0; i < vao.tex.size(); ++i) {
      auto& tex = vao.tex[i];
      tex.bind_material(shader, tex.type() == aiTextureType_DIFFUSE ? diff_n++ : spec_n++, i);
    }

    glBindVertexArray(vao.id());
    glDrawElements(GL_TRIANGLES, vao.ind(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }

}

template<>
void Renderer::draw(Shader& shader, Sprite& obj) {
  shader.set_mat4("proj", oproj_m);
  shader.set_mat4("model", obj.model_m);
  
  glBindTexture(GL_TEXTURE_2D, obj.texture.get().id());
  glActiveTexture(GL_TEXTURE0);
  shader.set_int("sprite_texture", 0);

  glBindVertexArray(q_VAO);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}


}
