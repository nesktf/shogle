#pragma once

#include <shogle/scene/entity.hpp>
#include <shogle/scene/camera.hpp>
#include <shogle/scene/tasker.hpp>

#include <shogle/render/sprite.hpp>

namespace ntf {

class sprite : public entity2D {
public:
  sprite(render::sprite* sprite, render::shader* shader, camera2D& cam);

public:
  virtual void draw() override;

public:
  inline void set_sprite(render::sprite* sprite) { _sprite = sprite; }
  inline void set_shader(render::shader* shader) { _shader = shader; }
  inline void set_cam(camera2D& cam) { _cam = cam; };
  inline void set_index(size_t i) { _index = i; }

  inline size_t index_count(void) { return _sprite->count(); }

  inline vec2 corrected_scale(float base = 1.0f) { return vec2{base*_sprite->aspect(), base}; }

public:
  bool use_screen_space {false};
  bool inverted_draw {false};
  color4 color {1.0f};

protected:
  virtual void update_shader(void);

private:
  render::sprite* _sprite;
  render::shader* _shader;
  camera2D& _cam;
  size_t _index {0};
};

struct dynamic_sprite : public tasker<sprite, dynamic_sprite> { 
  template<typename... Args>
  dynamic_sprite(Args&&... args) : 
    tasker(std::forward<Args>(args)...) {}
};

} // namespace ntf
