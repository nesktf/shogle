#pragma once

#include "scene/scene_object.hpp"

#include "render/sprite_renderer.hpp"

namespace ntf {

class Sprite : public SceneObj<Sprite, SpriteRenderer> {
public:
  struct SpritesheetInfo {
    size_t count {1};
    size_t rows{1}, cols{1};
    size_t index {0};
  };

public:
  Sprite(Texture* tex, Shader* sha);

  Sprite(Texture* tex, Shader* sha, size_t sprite_count, size_t sprite_cols);
  
protected:
  virtual void shader_update(Shader* shader, glm::mat4 model_m) override;
  virtual glm::mat4 model_m_gen(void) override;

public:
  glm::vec2 pos {0.0f};
  glm::vec2 scale {1.0f};
  float rot {0.0f};

  unsigned int layer {0};
  glm::vec4 color {1.0f};

  SpritesheetInfo info;

  inline void set_sprite_count(size_t count, size_t cols) {
    size_t rows = std::ceil((float)count/(float)cols);
    offset.z = 1.0f/(float)cols;
    offset.w = 1.0f/(float)rows;

    info.count = count;
    info.cols = cols;
    info.rows = rows;
  }

  inline void set_sprite_index(size_t i) {
    i = i % info.count; // don't overflow
    size_t curr_row = i / info.rows;
    size_t curr_col = i % info.cols;

    offset.x = (float)curr_col;
    offset.y = (float)curr_row;

    info.index = i;
  }

protected:
  glm::vec4 offset {glm::vec2{0.0f}, glm::vec2{1.0f}};
};

} // namespace ntf
