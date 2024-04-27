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
  virtual void update_shader(void);

public:
  inline void draw() { _sprite->draw(*_shader); }

  inline void set_sprite(render::sprite* sprite) { _sprite = sprite; }
  inline void set_shader(render::shader* shader) { _shader = shader; }
  inline void set_cam(Camera2D* cam) { _cam = cam; };

public:
  bool use_screen_space {false};
  bool draw_on_update {false};
  color4 color {1.0f};

private:
  render::sprite* _sprite;
  render::shader* _shader;
  Camera2D* _cam;
};

struct SpriteDynamic : public Tasker<Sprite> { 
  template<typename... Args>
  SpriteDynamic(Args&&... args) : 
    Tasker<Sprite>(std::forward<Args>(args)...) {};
};

} // namespace ntf
