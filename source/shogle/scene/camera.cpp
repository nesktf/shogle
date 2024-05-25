#include <shogle/scene/camera.hpp>

namespace ntf::shogle::scene {

camera2d::camera2d(vec2sz sz) :
  camera2d((float)sz.w, (float)sz.h) {}

camera2d::camera2d(float w, float h) {
  _ccom.viewport = {w, h};
  _ccom.zfar = 1.0f;
  _ccom.znear = -10.0f;
  update();
}

camera3d::camera3d(vec2sz sz) :
  camera3d((float)sz.w, (float)sz.h) {}

camera3d::camera3d(float w, float h) {
  _ccom.viewport = {w, h};
  update();
}

static inline mat4 cam_proj_ortho(vec2 viewport, float znear, float zfar) {
  return glm::ortho(
    0.0f, viewport.x,
    viewport.y, 0.0f,
    znear, zfar
  );
}

static inline mat4 cam_proj_persp(vec2 viewport, float znear, float zfar, float fov) {
  return glm::perspective(
    fov,
    viewport.x/viewport.y,
    znear, zfar
  );
}

static inline mat4 cam_view2d(vec2 origin, vec2 center, vec2 zoom, float rot) {
  mat4 view {1.0f};

  view = glm::translate(view, vec3{origin, 0.0f});
  view = glm::rotate(view, rot, vec3{0.0f, 0.0f, 1.0f});
  view = glm::scale(view, vec3{zoom, 1.0f});
  view = glm::translate(view, vec3{-center, 0.0f});

  return view;
}

static inline mat4 cam_view3d(vec3 pos, vec3 dir, vec3 up) {
  return glm::lookAt(
    pos,
    pos + dir,
    up
  );
}

void camera2d::update() {
  _ccom.proj = cam_proj_ortho(_ccom.viewport, _ccom.znear, _ccom.zfar);
  _ccom.view = cam_view2d(_origin, _center, _zoom, _rot);
}

void camera3d::update() {
  if (_use_ortho) {
    _ccom.proj = cam_proj_ortho(_ccom.viewport, _ccom.znear, _ccom.zfar);
  } else {
    _ccom.proj = cam_proj_persp(_ccom.viewport, _ccom.znear, _ccom.zfar, _fov);
  }
  _ccom.view = cam_view3d(_pos, _dir, _up);
}

} // namespace ntf::shogle::scene
