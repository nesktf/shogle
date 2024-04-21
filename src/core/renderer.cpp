#include "core/renderer.hpp"

#include "core/singleton.hpp"

namespace ntf {

struct SpriteQuad : public Singleton<SpriteQuad> {
  SpriteQuad();
  ~SpriteQuad();
  GLuint vao, vbo, ebo;
  GLuint vao_alt, vbo_alt, ebo_alt;
};

template<>
void SpriteRenderer::draw(void) {
  const auto* _texture = _res;
  const auto& _quad = SpriteQuad::instance();

  glBindTexture(GL_TEXTURE_2D, _texture->id());
  glActiveTexture(GL_TEXTURE0);

  glBindVertexArray(alt_draw ? _quad.vao_alt : _quad.vao);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

template<>
void ModelRenderer::draw(void) {
  const auto* _model = _res;

  for (const auto& mesh : _model->meshes) {
    size_t diff_n = 1;
    size_t spec_n = 1;
    for (size_t i = 0; i < mesh.materials.size(); ++i) {
      const auto& mat = mesh.materials[i];
      mat.bind_uniform(_shader, mat.type() == MaterialType::Diffuse ? diff_n++ : spec_n++, i);
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
  float quad_vert_inv[] = { // for framebuffers
    // coord      // tex_coord
    -0.5f, -0.5f, 0.0f, 1.0f,
     0.5f, -0.5f, 1.0f, 1.0f,
     0.5f,  0.5f, 1.0f, 0.0f,
    -0.5f,  0.5f, 0.0f, 0.0f
  };
  GLuint quad_ind[] = {
    0, 1, 2,
    0, 2, 3
  };

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vert), quad_vert, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_ind), quad_ind, GL_STATIC_DRAW);

  // Sprite shader needs a vec4f with 2d coords and tex coords
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
  glBindVertexArray(0);

  glGenVertexArrays(1, &vao_alt);
  glGenBuffers(1, &vbo_alt);
  glGenBuffers(1, &ebo_alt);

  glBindVertexArray(vao_alt);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_alt);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vert_inv), quad_vert_inv, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_alt);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_ind), quad_ind, GL_STATIC_DRAW);

  // Sprite shader needs a vec4f with 2d coords and tex coords
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
  glBindVertexArray(0);
}

SpriteQuad::~SpriteQuad() {
  glBindVertexArray(vao);
  glDisableVertexAttribArray(0);
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &ebo);
  glDeleteBuffers(1, &vbo);
}

} // namespace ntf
