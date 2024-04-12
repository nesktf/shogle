#include "scene/sprite.hpp"

#include "core/engine.hpp"
#include "core/log.hpp"

namespace ntf {

Sprite::Sprite(Texture* tex, Shader* sha) :
  SceneObj(tex, sha) {
    sprite.x = tex->w();
    sprite.y = tex->h();
    sprite.dx = tex->w();
    sprite.dy = tex->h();
    sprite.y0 = 0;
    sprite.x0 = 0;
    sprite.cols = 1;
    sprite.rows = 1;
    sprite.count = 1;
    curr_index = 0;
}

Sprite::Sprite(Spritesheet* sheet, std::string name, Shader* sha) :
  SceneObj(static_cast<Texture*>(sheet), sha),
  sprite(sheet->sprites.at(name)) { 
    offset.x = (float)sprite.dx/(float)(sprite.x*sprite.cols);
    offset.y = (float)sprite.dy/(float)(sprite.y*sprite.rows);
    set_index(0); 
}

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
  shader->unif_vec4("texture_color", color);
  shader->unif_vec4("texture_offset", offset);
  shader->unif_int("texture_sampler", 0);
}

void Sprite::set_index(size_t i) {
  i = i % sprite.count; // don't overflow
  size_t curr_row = i / sprite.cols;
  size_t curr_col = i % sprite.cols;

  offset.z = (float)(sprite.x0+(curr_col*sprite.dx))/(float)(sprite.x*sprite.cols);
  offset.w = (float)(sprite.y0+(curr_row*sprite.dy))/(float)(sprite.y*sprite.rows);

  curr_index = i;
}

} // namespace ntf
