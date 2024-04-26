#pragma once

#include <shogle/core/types.hpp>
#include <shogle/scene/scene.hpp>

namespace ntf {

class Camera2D : public Scene::Object {
public:
  struct data_t {
    struct proj {
      mat4 mat {1.0f};
      bool upd {true};

      vec2 viewport {800.0f, 600.0f};
      size_t layer_count {10};
    };
    struct view {
      mat4 mat {1.0f};
      bool upd {true};

      vec2 center {0.0f, 0.0f};
      vec2 origin {400.0f, 300.0f};
      vec2 zoom {1.0f};
      float rot {1.0f};
    };
  };
  
public:
  Camera2D() = default;
  Camera2D(data_t::proj proj);
  Camera2D(data_t::proj proj, data_t::view view);

public:
  void update(float) final;

public:
  inline Camera2D& set_viewport(vec2 viewport) {
    _proj.viewport = viewport;
    _proj.upd = true;
    _view.origin = viewport*0.5f;
    _view.upd = true;
    return *this;
  }

  inline Camera2D& set_layer_count(size_t layer_count) {
    _proj.layer_count = layer_count;
    _proj.upd = true;
    return *this;
  }

  inline Camera2D& set_center(vec2 center) {
    _view.center = center;
    _view.upd = true;
    return *this;
  }

  inline Camera2D& set_zoom(vec2 zoom) {
    _view.zoom = zoom;
    _view.upd = true;
    return *this;
  }

  inline Camera2D& set_zoom(float zoom) {
    _view.zoom = vec2{zoom};
    _view.upd = true;
    return *this;
  }

  inline Camera2D& set_rot(float rot) {
    _view.rot = rot;
    _view.upd = true;
    return *this;
  }

public:
  inline vec2 viewport(void) { return _proj.viewport; }
  inline size_t layer_count(void) { return _proj.layer_count; }

  inline vec2 center(void) { return _view.center; }
  inline vec2 zoom(void) { return _view.zoom; }
  inline float rot(void) { return _view.rot; }

  inline mat4 proj(void) { return _proj.mat; }
  inline mat4 view(void) { return _view.mat; }

private:
  data_t::proj _proj;
  data_t::view _view;
};

class Camera3D : public Scene::Object {
public:
  struct data_t {
    struct proj {
      mat4 mat {1.0f};
      bool upd {true};
      bool use_ortho {false};

      vec2 viewport {800.0f, 600.0f};
      vec2 draw_dist {0.1f, 100.0f};
      float fov {M_PIf*0.25f};
    };
    struct view {
      mat4 mat {1.0f};
      bool upd {true};

      vec3 pos {0.0f};
      vec3 up {0.0f, 1.0f, 0.0f};
      vec3 dir {0.0f, 0.0f, -1.0f};

      // float yaw {-M_PIf*0.5f};
      // float pitch {0.0f};
    };
  };

public:
  Camera3D() = default;
  Camera3D(data_t::proj proj);
  Camera3D(data_t::proj proj, data_t::view view);

public:
  void update(float) final;

public:
  inline Camera3D& enable_ortho(bool flag = true) {
    _proj.use_ortho = flag;
    _proj.upd = true;
    return *this;
  }
  
  inline Camera3D& set_viewport(vec2 viewport) {
    _proj.viewport = viewport;
    _proj.upd = true;
    return *this;
  }

  inline Camera3D& set_draw_dist(float zfar, float znear = 0.1f) {
    _proj.draw_dist = vec2{znear, zfar};
    _proj.upd = true;
    return *this;
  }

  inline Camera3D& set_fov(float fov) {
    _proj.fov = fov;
    _proj.upd = true;
    return *this;
  }

  inline Camera3D& set_pos(vec3 pos) {
    _view.pos = pos;
    _view.upd = true;
    return *this;
  }

  inline Camera3D& set_dir(vec3 dir) {
    _view.dir = dir;
    _view.upd = true;
    return *this;
  }

public:
  inline vec2 viewport(void) { return _proj.viewport; }
  inline float zfar(void) { return _proj.draw_dist.x; }
  inline float znear(void) { return _proj.draw_dist.y; }
  inline float fov(void) { return _proj.fov; }

  inline vec3 pos(void) { return _view.pos; }
  inline vec3 dir(void) { return _view.dir; }

  inline mat4 proj(void) { return _proj.mat; }
  inline mat4 view(void) { return _view.mat; }

private:
  data_t::proj _proj;
  data_t::view _view;
};

} // namespace ntf
