#include <shogle/scene/camera.hpp>

namespace ntf::shogle::scene {

static mat4 gen_proj_ortho(vec2 viewport, float znear, float zfar) {
  return glm::ortho(
    0.0f, viewport.x,
    viewport.y, 0.0f,
    znear, zfar
  );
}

static mat4 gen_proj_persp(vec2 viewport, float znear, float zfar, float fov) {
  return glm::perspective(
    fov,
    viewport.x/viewport.y,
    znear, zfar
  );
}

static mat4 gen_view2d(vec2 origin, vec2 center, vec2 zoom, float rot) {
  mat4 view {1.0f};

  view = glm::translate(view, vec3{origin, 0.0f});
  view = glm::rotate(view, rot, vec3{0.0f, 0.0f, 1.0f});
  view = glm::scale(view, vec3{zoom, 1.0f});
  view = glm::translate(view, vec3{-center, 0.0f});

  return view;
}

static mat4 gen_view3d(vec3 pos, vec3 dir, vec3 up) {
  return glm::lookAt(
    pos,
    pos + dir,
    up
  );
}

void camera2d::update() {
  _proj = gen_proj_ortho(_viewport, _znear, _zfar);
  _view = gen_view2d(_origin, _center, _zoom, _rot);
}

void camera3d::update() {
  if (_use_ortho) {
    _proj = gen_proj_ortho(_viewport, _znear, _zfar);
  } else {
    _proj = gen_proj_persp(_viewport, _znear, _zfar, _fov);
  }
  _view = gen_view3d(_pos, _dir, _up);
}


} // namespace ntf::shogle::scene
