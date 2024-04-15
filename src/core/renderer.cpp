#include "core/renderer.hpp"

#include "core/singleton.hpp"

namespace ntf {

struct SpriteQuad : public Singleton<SpriteQuad> {
  SpriteQuad();
  ~SpriteQuad();
  GLuint VAO, VBO, EBO;
};

template<>
void SpriteRenderer::draw(void) {
  const auto* _texture = _res;
  const auto& _quad = SpriteQuad::instance();

  glBindTexture(GL_TEXTURE_2D, _texture->id());
  glActiveTexture(GL_TEXTURE0);

  glBindVertexArray(_quad.VAO);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

template<>
void ModelRenderer::draw(void) {
  const auto* _model = _res;

  for (const auto& mesh : _model->meshes) {
    size_t diff_n = 1;
    size_t spec_n = 1;
    for (size_t i = 0; i < mesh.tex.size(); ++i) {
      const auto& tex = mesh.tex[i];
      tex.bind_material(*_shader, tex.type() == aiTextureType_DIFFUSE ? diff_n++ : spec_n++, i);
    }

    glBindVertexArray(mesh.id());
    glDrawElements(GL_TRIANGLES, mesh.ind(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }
}

SpriteQuad::SpriteQuad() {
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

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vert), quad_vert, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_ind), quad_ind, GL_STATIC_DRAW);

  // Sprite shader needs a vec4f with 2d coords and tex coords
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
  glBindVertexArray(0);
}

SpriteQuad::~SpriteQuad() {
  glBindVertexArray(VAO);
  glDisableVertexAttribArray(0);
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &EBO);
  glDeleteBuffers(1, &VBO);
}

} // namespace ntf
