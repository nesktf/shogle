#pragma once

#include <shogle/core/types.hpp>

namespace ntf::shogle::scene {

/**
 * @brief Camera view implementation
 *
 * @tparam dim_t Dimension type
 * @tparam T Actual camera type (for method chaining)
 */
template<typename dim_t, typename T>
class camera_view;

/**
 * @brief Camera abstraction. Generates view and projection matrices.
 *
 * @tparam dim_t dimension type (vec2/vec3)
 */
template<typename dim_t>
class camera : public camera_view<dim_t, camera<dim_t>> {
public:
  camera(vec2 viewport);
  camera(float w, float h);

public:
  /**
   * @brief Regenerates the view and projection matrices
   */
  void update();

public:
  /**
   * @brief Set the camera viewport
   *
   * @param viewport Viewport size vec2
   * @return this
   */
  inline camera& set_viewport(vec2 viewport);

  /**
   * @brief Set the camera viewport. Alias for set_viewport(vec2{w,h})
   *
   * @param w viewport width
   * @param h viewport height
   * @return 
   */
  inline camera& set_viewport(float w, float h);

  /**
   * @brief Set znear
   *
   * @param znear 
   * @return this
   */
  inline camera& set_znear(float znear);

  /**
   * @brief Set zfar
   *
   * @param zfar 
   * @return this
   */
  inline camera& set_zfar(float zfar);

public:
  /**
   * @brief Projection matrix getter
   *
   * @return Projection matrix
   */
  mat4 proj() const { return _proj; }

  /**
   * @brief View matrix getter
   *
   * @return View matrix
   */
  mat4 view() const { return _view; }

  /**
   * @brief znear getter
   *
   * @return znear scalar
   */
  float znear() const { return _znear; }

  /**
   * @brief zfar getter
   *
   * @return zfar scalar
   */
  float zfar() const { return _zfar; }

  /**
   * @brief Viewport getter
   *
   * @return Viewport vec2
   */
  vec2 vport() const { return _viewport; }

private:
  mat4 _proj {1.0f};
  mat4 _view {1.0f};
  float _znear {0.1f};
  float _zfar {100.0f};
  vec2 _viewport;
};

template<typename T>
class camera_view<vec2, T> {
protected:
  camera_view(vec2 viewport);

public:
  /**
   * @brief Set camera position (view target)
   *
   * @param center Position coordinates (in world space)
   * @return this
   */
  inline T& set_position(vec2 center);

  /**
   * @brief Set camera position (view target). Alias for set_center(vec2{x,y})
   *
   * @param x Position X coordinate
   * @param y Position Y coordinate
   * @return this
   */
  inline T& set_position(float x, float y);

  /**
   * @brief Set camera zoom
   *
   * @param zoom Zoom vec2
   * @return this
   */
  inline T& set_zoom(vec2 zoom);

  /**
   * @brief Set camera zoom. Alias for set_zoom(vec2{x,y})
   *
   * @param x Zoom X component
   * @param y Zoom Y component
   * @return this
   */
  inline T& set_zoom(float x, float y);

  /**
   * @brief Set camera zoom. Alias for set_zoom(vec2{zoom})
   *
   * @param zoom Zoom scalar
   * @return this
   */
  inline T& set_zoom(float zoom);

  /**
   * @brief Set camera rotation. Follows left hand rule.
   *
   * @param rot Rotation angle (in radians)
   * @return this
   */
  inline T& set_rotation(float rot);

public:
  /**
   * @brief Center coordinates getter
   *
   * @return Center coordinates vec2
   */
  vec2 center() const { return _pos; }

  /**
   * @brief Zoom getter
   *
   * @return Zoom vec2
   */
  vec2 zoom() const { return _zoom; }

  /**
   * @brief Rotation getter
   *
   * @return Rotation angle (in radians)
   */
  float rot() const { return _rot; }

protected:
  vec2 _pos {0.0f};
  vec2 _zoom {1.0f};
  float _rot {0.0f};
  vec2 _origin;
};

template<typename T>
class camera_view<vec3, T> {
protected:
  camera_view(vec2 viewport);

public:
  /**
   * @brief Set camera position
   *
   * @param pos vec3 with position coordinates
   * @return this
   */
  inline T& set_position(vec3 pos);

  /**
   * @brief Set camera position. Alias for set_position(vec3{x,y,z})
   *
   * @param x Position X coordinate
   * @param y Position Y coordinate
   * @param z Position Z coordinate
   * @return this
   */
  inline T& set_position(float x, float y, float z);

  /**
   * @brief Set camera view direction
   *
   * @param dir Direction vec3 (normalized)
   * @return this
   */
  inline T& set_direction(vec3 dir);

  /**
   * @brief Set camera view direction. Alias for set_direction(vec3{x,y,z})
   *
   * @param x Direction X component
   * @param y Direction Y component
   * @param z Direction Z component
   * @return this
   */
  inline T& set_direction(float x, float y, float z);

  /**
   * @brief Set camera fov
   *
   * @param fov Fov angle (in radians)
   * @return this
   */
  inline T& set_fov(float fov);

  /**
   * @brief Enable perspective projection
   *
   * @return this
   */
  inline T& use_perspective();

  /**
   * @brief Enable orthographic projection
   *
   * @return this
   */
  inline T& use_orthographic();

public:
  /**
   * @brief Check if camera is using perspective projection
   *
   * @return bool
   */
  bool is_persp() const { return _use_persp; }

  /**
   * @brief Check if camera is using orthographic projection
   *
   * @return bool
   */
  bool is_ortho() const { return !_use_persp; }

  /**
   * @brief Position getter
   *
   * @return Position vec3
   */
  vec3 pos() const { return _pos; }

  /**
   * @brief Direction getter
   *
   * @return Direction vec3
   */
  vec3 dir() const { return _dir; }

  /**
   * @brief Up direction getter
   *
   * @return Up vec3
   */
  vec3 up() const { return _up; }

  /**
   * @brief Fov getter
   *
   * @return Fov angle (in radians)
   */
  float fov() const { return _fov; }

protected:
  bool _use_persp {true};
  vec3 _pos {0.0f};
  vec3 _dir {0.0f, 0.0f, -1.0f};
  vec3 _up {0.0f, 1.0f, 0.0f};
  float _fov {M_PIf*0.25f};
};

/**
 * @brief Generate orthographic projection matrix
 *
 * @param viewport Viewport size vec2
 * @param znear
 * @param zfar 
 * @return Projection matrix
 */
mat4 proj_ortho(vec2 viewport, float znear, float zfar);

/**
 * @brief Generate perspective projection matrix
 *
 * @param viewport Viewport size vec2
 * @param znear 
 * @param zfar 
 * @param fov Fov angle (in radians)
 * @return Projection matrix
 */
mat4 proj_persp(vec2 viewport, float znear, float zfar, float fov);

/**
 * @brief Generate 2D view matrix
 *
 * @param origin Camera origin in screen space. Usually viewport/2
 * @param center Camera position (view target) in world space
 * @param zoom 
 * @param rot Rotation angle (in radians)
 * @return View matrix
 */
mat4 view2d(vec2 origin, vec2 pos, vec2 zoom, float rot);

/**
 * @brief Generate 3D view matrix
 *
 * @param pos Camera position
 * @param dir Camera view direction
 * @param up Camera up direction
 * @return View matrix
 */
mat4 view3d(vec3 pos, vec3 dir, vec3 up);


/**
  * @brief 2D camera abstraction
  */
using camera2d = camera<vec2>;

/**
  * @brief 3D camera abstraction
  */
using camera3d = camera<vec3>;

} // namespace ntf::shogle::scene

#ifndef CAMERA_INL_HPP
#include <shogle/scene/camera.inl.hpp>
#endif
