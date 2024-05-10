#pragma once

#include <shogle/scene/entity.hpp>
#include <shogle/scene/camera.hpp>
#include <shogle/scene/tasker.hpp>

#include <shogle/render/sprite.hpp>
#include <shogle/res/global.hpp>

namespace ntf {

class sprite : public entity2d {
public:
  sprite(wptr<render::sprite> sprite);
  sprite(wptr<render::sprite> sprite, wptr<render::shader> shader);
  sprite(wptr<render::sprite> sprite, wptr<camera2d> cam);
  sprite(wptr<render::sprite> sprite, wptr<camera2d> cam, wptr<render::shader> shader);

public:
  virtual void draw() override;

public:
  inline void set_sprite(wptr<render::sprite> sprite) { _sprite = sprite; }
  inline void set_cam(wptr<camera2d> cam) { _cam = cam; };
  inline void set_shader(wptr<render::shader> shader) { _shader = shader; }

  inline void set_index(size_t i) { _index = i%index_count(); }
  inline void next_index(void) { set_index(++_index); }
  inline size_t index_count(void) { return _sprite->count(); }
  inline size_t index(void) { return _index; }

  inline vec2 corrected_scale(float base = 1.0f) { return vec2{base*_sprite->aspect(), base}; }
  inline void toggle_inverted_draw(bool flag) { _inverted_draw = flag; }

public:
  color4 color {1.0f};

protected:
  virtual void update_shader(void);

private:
  wptr<render::sprite> _sprite;
  wptr<render::shader> _shader {res::def_sprite_sh.get()};
  wptr<camera2d> _cam {&res::def_cam2d};
  size_t _index {0};
  bool _inverted_draw {false};
};

struct dynamic_sprite : public tasker<sprite, dynamic_sprite> { 
  template<typename... Args>
  dynamic_sprite(Args&&... args) : 
    tasker(std::forward<Args>(args)...) {}
};

} // namespace ntf
