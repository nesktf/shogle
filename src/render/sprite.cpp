#include "render/sprite.hpp"

#include "shogle.hpp"
#include "singleton.hpp"
#include "log.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace ntf::shogle::render {

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

    Log::debug("[Sprite] Sprite rendrerer initialized (vao-id: {})", q_VAO);
  }
  ~SpriteRenderer() {
    GLint vao = this->q_VAO;
    glBindVertexArray(this->q_VAO);
    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &this->q_VAO);
    glDeleteBuffers(1, &this->q_EBO);
    glDeleteBuffers(1, &this->q_VBO);
    Log::debug("[Sprite] Sprite renderer deleted (vao-id: {})", vao);
  }
  void draw(void) {
    glBindVertexArray(q_VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }
private:
  GLuint q_VAO, q_VBO, q_EBO;
public:
};

void Sprite::draw(void) {
  const auto& eng = Engine::instance();
  const auto& shader = shader_ref.get();
  const auto& texture = texture_ref.get();
  auto& renderer = SpriteRenderer::instance();
  
  shader.use();
  shader.unif_mat4("proj", eng.proj2d);
  shader.unif_mat4("model", this->model_m);
  shader.unif_vec4("sprite_color", glm::vec4{1.0f});
  shader.unif_int("sprite_texture", 0);

  glBindTexture(GL_TEXTURE_2D, texture.id());
  glActiveTexture(GL_TEXTURE0);

  renderer.draw();
}

glm::mat4 Sprite::model_transform(TransformData transform) {
  glm::mat4 mat{1.0f};

  mat = glm::translate(mat, glm::vec3{transform.pos.x, transform.pos.y, 0.0f});
  mat = glm::rotate(mat, glm::radians(transform.rot.z), glm::vec3{0.0f, 0.0f, 1.0f});
  mat = glm::scale(mat, glm::vec3{transform.scale.x, transform.scale.y, 1.0f});
  mat = glm::scale(mat, glm::vec3{transform.scale.x, transform.scale.y, 0.0f});

  return mat;
}

} // namespace ntf::shogle
