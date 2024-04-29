#pragma once

#include <shogle/scene/entity.hpp>
#include <shogle/scene/camera.hpp>

#include <shogle/render/sprite.hpp>

#include <shogle/core/tasker.hpp>

namespace ntf {

class Sprite : public Entity2D {
public:
  Sprite(render::sprite* sprite, render::shader* shader, Camera2D* cam = &Camera2D::default_cam);

public:
  virtual void update(float dt) override;

public:
  inline void draw() { _sprite->draw(*_shader, _index, inverted_draw); }

public:
  inline void set_sprite(render::sprite* sprite) { _sprite = sprite; }
  inline void set_shader(render::shader* shader) { _shader = shader; }
  inline void set_cam(Camera2D* cam) { _cam = cam; };
  inline void set_index(size_t i) { _index = i; }

  inline size_t index_count(void) { return _sprite->count(); }

  inline vec2 corrected_scale(float base = 1.0f) { return vec2{base*_sprite->aspect(), base}; }

public:
  bool use_screen_space {false};
  bool draw_on_update {false};
  bool inverted_draw {false};
  color4 color {1.0f};

protected:
  virtual void update_shader(void);

private:
  render::sprite* _sprite;
  render::shader* _shader;
  Camera2D* _cam;
  size_t _index {0};
};

struct SpriteDynamic : public Tasker<Sprite, SpriteDynamic> { 
  template<typename... Args>
  SpriteDynamic(Args&&... args) : 
    Tasker(std::forward<Args>(args)...) {}
};

} // namespace ntf
