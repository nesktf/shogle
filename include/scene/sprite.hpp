#pragma once

#include "scene/scene_object.hpp"
#include "scene/camera2d.hpp"

#include "core/renderer.hpp"

#include "res/spritesheet.hpp"

namespace ntf {

class SpriteImpl : public SpriteRenderer, public SceneObj {
protected:
  SpriteImpl(const Texture* tex, const Shader* sha);
  SpriteImpl(const Spritesheet* sheet, std::string name, const Shader* sha);

public:
  virtual void update(float dt) override;
  void udraw(float dt) { update(dt); draw(); }

public:
  void set_index(size_t i);
  void next_index(void) { set_index(_index+1); }
  size_t index(void) { return _index; }

public:
  inline vec2 corrected_scale(float base = 1.0f) { return vec2{base*aspect(), base}; }
  inline float aspect(void) {
    return static_cast<float>(_sprite.dx*_sprite.rows)/static_cast<float>(_sprite.dy*_sprite.cols);
  }

protected:
  virtual void _shader_update(void);
  mat4 _gen_model(void) override;

protected:
  SpriteData _sprite;
  size_t _index;
  vec4 _offset {vec2{1.0f}, vec2{0.0f}};

public:
  Camera2D* cam;

  vec2 pos {0.0f}, scale{1.0f};
  float rot {0.0f};
  uint layer {0};

  color4 color {1.0f};
  bool use_screen_space {false};
};

struct Sprite : public TaskedObj<Sprite, SpriteImpl> {
  Sprite(const Texture* texture, const Shader* shader) :
    TaskedObj(texture, shader) {}
  Sprite(const Spritesheet* sheet, std::string name, const Shader* shader) :
    TaskedObj(sheet, name, shader) {}
};

} // namespace ntf
