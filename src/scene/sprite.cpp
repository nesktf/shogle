#include "scene/sprite.hpp"

#include "core/engine.hpp"

namespace ntf {

Sprite::Sprite(Texture* tex, Shader* sha) :
  SceneObj(tex, sha) {}

glm::mat4 Sprite::model_m_gen(void) {
  glm::mat4 mat{1.0f};

  mat = glm::translate(mat, glm::vec3{pos, (float)layer});
  mat = glm::rotate(mat, rot, glm::vec3{0.0f, 0.0f, 1.0f});
  mat = glm::scale(mat, glm::vec3{scale, 1.0f});

  return mat;
}

void Sprite::shader_update(Shader* shader, glm::mat4 model_m) {
  const auto& eng = Shogle::instance();

  shader->use();
  shader->unif_mat4("proj", eng.proj2d);
  shader->unif_mat4("model", model_m);
  shader->unif_vec4("sprite_color", color);
  shader->unif_int("sprite_texture", 0);
}

} // namespace ntf
