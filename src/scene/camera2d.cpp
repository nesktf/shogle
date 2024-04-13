#include "scene/camera2d.hpp"

namespace ntf {

Camera2D::Camera2D(proj_info p_info) :
  _proj_info(p_info),
  _view_info(view_info{
    .center = vec2{0.0f, 0.0f},
    .origin = vec2{p_info.viewport.x*0.5f, p_info.viewport.y*0.5f},
    .zoom = vec2{1.0f, 1.0f},
    .rot = 0.0f
  }) { 
    _proj = _gen_proj(_proj_info); 
    _view = _gen_view(_view_info);
}

Camera2D::Camera2D(proj_info p_info, view_info v_info) :
  _proj_info(p_info),
  _view_info(v_info) {
    _proj = _gen_proj(_proj_info); 
    _view = _gen_view(_view_info);
}

mat4 Camera2D::_gen_proj(proj_info v_info) {
  return glm::ortho(
    0.0f, v_info.viewport.x,
    v_info.viewport.y, 0.0f,
    -(float)v_info.layer_count, 1.0f
  );
}

mat4 Camera2D::_gen_view(view_info info) {
  mat4 view{1.0f};

  view = glm::translate(view, vec3{info.origin, 0.0f});
  view = glm::rotate(view, info.rot, vec3{0.0f, 0.0f, 1.0f});
  view = glm::scale(view, vec3{info.zoom, 1.0f});
  view = glm::translate(view, vec3{-info.center, 0.0f});

  return view;
}

} // namespace ntf
