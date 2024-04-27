#pragma once

#include <shogle/core/types.hpp>
#include <shogle/scene/scene.hpp>

namespace ntf {

class Camera2D : public Scene::Object {
public:
  Camera2D();

public:
  void update(float) final;

public:
  inline Camera2D& set_viewport(vec2 viewport) {
    _viewport = viewport;
    _origin = viewport*0.5f;
    return *this;
  }

  inline Camera2D& set_layer_count(size_t layer_count) {
    _layer_count = layer_count;
    return *this;
  }

  inline Camera2D& set_center(vec2 center) {
    _center = center;
    return *this;
  }

  inline Camera2D& set_zoom(vec2 zoom) {
    _zoom = zoom;
    return *this;
  }

  inline Camera2D& set_zoom(float zoom) {
    _zoom = vec2{zoom};
    return *this;
  }

  inline Camera2D& set_rot(float rot) {
    _rot = rot;
    return *this;
  }

public:
  inline vec2 viewport(void) { return _viewport; }
  inline size_t layer_count(void) { return _layer_count; }

  inline vec2 center(void) { return _center; }
  inline vec2 zoom(void) { return _zoom; }
  inline float rot(void) { return _rot; }

  inline mat4 proj(void) { return _proj; }
  inline mat4 view(void) { return _view; }

public:
  static Camera2D default_cam;

private:
  mat4 _proj {1.0f};
  vec2 _viewport {800.0f, 600.0f};
  size_t _layer_count {10};

  mat4 _view {1.0f};
  vec2 _center {0.0f, 0.0f};
  vec2 _origin {400.0f, 300.0f};
  vec2 _zoom {1.0f};
  float _rot {1.0f};
};

class Camera3D : public Scene::Object {
public:
  Camera3D();

public:
  void update(float) final;

public:
  inline Camera3D& use_ortho(bool flag) {
    _use_ortho = flag;
    return *this;
  }
  
  inline Camera3D& set_viewport(vec2 viewport) {
    _viewport = viewport;
    return *this;
  }

  inline Camera3D& set_draw_dist(float zfar, float znear = 0.1f) {
    _draw_dist = vec2{znear, zfar};
    return *this;
  }

  inline Camera3D& set_fov(float fov) {
    _fov = fov;
    return *this;
  }

  inline Camera3D& set_pos(vec3 pos) {
    _pos = pos;
    return *this;
  }

  inline Camera3D& set_dir(vec3 dir) {
    _dir = dir;
    return *this;
  }

public:
  inline vec2 viewport(void) { return _viewport; }
  inline float zfar(void) { return _draw_dist.x; }
  inline float znear(void) { return _draw_dist.y; }
  inline float fov(void) { return _fov; }

  inline vec3 pos(void) { return _pos; }
  inline vec3 dir(void) { return _dir; }

  inline mat4 proj(void) { return _proj; }
  inline mat4 view(void) { return _view; }

public:
  static Camera3D default_cam;

private:
  mat4 _proj {1.0f};
  mat4 _view {1.0f};

  bool _use_ortho {false};
  vec2 _viewport {800.0f, 600.0f};
  vec2 _draw_dist {0.1f, 100.0f};
  float _fov {M_PIf*0.25f};

  vec3 _pos {0.0f};
  vec3 _up {0.0f, 1.0f, 0.0f};
  vec3 _dir {0.0f, 0.0f, -1.0f};
  // float yaw {-M_PIf*0.5f};
  // float pitch {0.0f};
};

} // namespace ntf
