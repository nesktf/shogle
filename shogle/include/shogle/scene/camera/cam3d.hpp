#pragma once

#include <shogle/core/types.hpp>

namespace ntf {

class Camera3D {
public:
  struct proj_info {
    vec2 viewport{800.0f, 600.0f};
    vec2 draw_dist{0.1f, 100.0f};
    float fov {M_PIf*0.25f};
  };
  struct view_info {
    vec3 pos{0.0f, 0.0f, 0.0f};
    vec3 up{0.0f, 1.0f, 0.0f};
    vec3 dir_vec{0.0f, 0.0f, -1.0f};
  };
  struct cam_dir {
    float yaw {-M_PIf*0.5f};
    float pitch{0.0f};
  }; // for convenience

public:
  Camera3D() = default;
  Camera3D(proj_info proj_info);
  Camera3D(proj_info proj_info, view_info view_info);
  
public:
  void upd_target(vec2 screen_pos, float sensitivity);

public:
  inline void set_viewport(vec2 viewport) {
    _proj_info.viewport = viewport;
    _proj = _gen_proj(_proj_info);
  }

  inline void set_draw_dist(vec2 draw_dist) {
    _proj_info.draw_dist = draw_dist;
    _proj = _gen_proj(_proj_info);
  }

  inline void set_draw_dist(float z_far) {
    _proj_info.draw_dist.y = z_far;
    _proj = _gen_proj(_proj_info);
  }

  inline void set_fov(float fov) {
    _proj_info.fov = fov;
    _proj = _gen_proj(_proj_info);
  }

  inline void set_pos(vec3 pos) {
    _view_info.pos = pos;
    _view = _gen_view(_view_info);
  }

  inline void set_up(vec3 up) {
    _view_info.up = up;
    _view = _gen_view(_view_info);
  }

  inline void set_dir(vec3 dir_vec) {
    _view_info.dir_vec = dir_vec;
    _view = _gen_view(_view_info);
  }

  inline void set_dir(cam_dir dir) {
    _view_info.dir_vec = dir_to_vec(dir);
    _view = _gen_view(_view_info);
  }

public:
  proj_info proj(void) const { return _proj_info; }
  view_info view(void) const { return _view_info; }
  cam_dir dir(void) const { return _cam_dir; } 

  mat4 proj_mat(void) const { return _proj; }
  mat4 view_mat(void) const { return _view; }
  vec3 view_pos(void) const { return _view_info.pos; } // for convenience

public:
  static vec3 dir_to_vec(cam_dir dir);
  static cam_dir vec_to_dir(vec3 dir_vec, vec3 up_vec);

private:
  static mat4 _gen_proj(proj_info proj);
  static mat4 _gen_view(view_info view);

private:
  mat4 _proj{1.0f};
  mat4 _view{1.0f};

  proj_info _proj_info{};
  view_info _view_info{};

  cam_dir _cam_dir{};
  vec2 _last_screen_pos{0.0f};
  bool _first_movement{true};
};

} // namespace ntf
