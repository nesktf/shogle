#pragma once

#include "core/types.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace ntf {

class Camera2D {
public:
  struct proj_info {
    vec2 viewport{800.0f, 600.0f};
    size_t layer_count{10};
  };
  struct view_info {
    vec2 center{0.0f, 0.0f};
    vec2 origin{400.0f, 300.0f}; vec2 zoom{1.0f, 1.0f};
    float rot{0.0f};
  };

public:
  Camera2D() = default;
  Camera2D(proj_info p_info);
  Camera2D(proj_info p_info, view_info v_info);

public:
  inline void set_viewport(vec2 viewport) {
    _proj_info.viewport = viewport;
    _proj = _gen_proj(_proj_info);
  }

  inline void set_layer_count(size_t layer_count) {
    _proj_info.layer_count = layer_count;
    _proj = _gen_proj(_proj_info);
  }

  inline void set_origin(vec2 origin) {
    _view_info.origin = origin;
    _view = _gen_view(_view_info);
  }

  inline void set_center(vec2 center) {
    _view_info.center = center;
    _view = _gen_view(_view_info);
  }

  inline void set_zoom(vec2 zoom) {
    _view_info.zoom = zoom;
    _view = _gen_view(_view_info);
  }

  inline void set_zoom(float zoom) {
    _view_info.zoom = vec2{zoom};
    _view = _gen_view(_view_info);
  }

  inline void set_rot(float rot) {
    _view_info.rot = rot;
    _view = _gen_view(_view_info);
  }

public:
  inline proj_info proj(void) const { return _proj_info; }
  inline view_info view(void) const { return _view_info; }

  inline mat4 view_mat() const { return _view; }
  inline mat4 proj_mat() const { return _proj; }

private:
  static mat4 _gen_proj(proj_info p_info);
  static mat4 _gen_view(view_info v_info);

private:
  mat4 _proj{1.0f};
  mat4 _view{1.0f};
  proj_info _proj_info{};
  view_info _view_info{};
};

} // namespace ntf
