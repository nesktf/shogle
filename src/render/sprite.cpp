#include "render/sprite.hpp"

#include "singleton.hpp"
#include "log.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace ntf::shogle {

class SpriteRenderer : public Singleton<SpriteRenderer> {
public:
  SpriteRenderer() {
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

    // TODO: Update proj matrix on screen resize
    this->proj = glm::ortho(0.0f, 600.0f, 800.0f, 0.0f, -1.0f, 1.0f);

    log::debug("[Sprite] Created sprite renderer");
  }
  ~SpriteRenderer() {
    GLint vao = this->q_VAO;
    glBindVertexArray(this->q_VAO);
    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &this->q_VAO);
    glDeleteBuffers(1, &this->q_EBO);
    glDeleteBuffers(1, &this->q_VBO);
    log::debug("[Sprite] Deleted sprite renderer (VAO id: {})", vao);
  }
  void draw(void) {
    glBindVertexArray(q_VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }
private:
  GLuint q_VAO, q_VBO, q_EBO;
public:
  glm::mat4 proj;
};

void Sprite::draw(void) {
  auto& renderer = SpriteRenderer::instance();
  shader->unif_mat4("proj", renderer.proj);
  shader->unif_mat4("model", model_m);

  glBindTexture(GL_TEXTURE_2D, texture->id());
  glActiveTexture(GL_TEXTURE0);
  shader->unif_int("sprite_texture", 0);

  renderer.draw();
}

} // namespace ntf::shogle