#pragma once

#include "scene/scene_object.hpp"
#include "scene/camera2d.hpp"

#include "render/sprite_renderer.hpp"

#include "res/spritesheet.hpp"

namespace ntf {

class Sprite : public SceneObj<Sprite, SpriteRenderer> {
public:
  Sprite(Texture* tex, Shader* sha);
  Sprite(Spritesheet* sheet, std::string name, Shader* sha);

protected:
  virtual void shader_update(Shader* shader, glm::mat4 model_m) override;
  virtual glm::mat4 model_m_gen(void) override;

public:
  inline vec2 corrected_scale(float base = 1.0f) {
    return {base*aspect(), base};
  }

  inline float aspect() {
    return ((float)sprite.dx/(float)sprite.cols)/((float)sprite.dy/(float)sprite.rows);
  }

public:
  void set_index(size_t i);

  inline void next_index(void) {
    set_index(curr_index+1);
  }

public:
  Camera2D* cam;
  vec2 pos {0.0f};
  vec2 scale {1.0f};
  float rot {0.0f};

  uint layer {0};
  vec4 color {1.0f};

  SpriteData sprite;
  size_t curr_index;
  vec4 offset{vec2{1.0f}, vec2{0.0f}};
};

} // namespace ntf
