#pragma once

#include "scene/scene_object.hpp"

#include "render/sprite_renderer.hpp"

namespace ntf {

class Sprite : public SceneObj<Sprite, SpriteRenderer> {
public:
  Sprite(Texture* tex, Shader* sha);
  
protected:
  virtual void shader_update(Shader* shader, glm::mat4 model_m) override;
  virtual glm::mat4 model_m_gen(void) override;

public:
  glm::vec2 pos {0.0f};
  glm::vec2 scale {1.0f};
  float rot {0.0f};
  unsigned int layer {0};
  glm::vec4 color {1.0f};
};

} // namespace ntf
