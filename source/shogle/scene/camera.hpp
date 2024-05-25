#pragma once

#include <shogle/core/types.hpp>

namespace ntf::shogle::scene {

/**
 * @class cam_common
 * @brief Common camera struct
 *
 */
struct cam_common {
  mat4 proj{1.0f}, view{1.0f};
  vec2 viewport{800.0f, 600.0f};
  float znear{0.1f}, zfar{100.0f};
};

/**
 * @class camera2d
 * @brief 2D Camera
 *
 */
class camera2d {
public:
  /**
   * @brief Build using vector for viewport
   *
   * @param sz Size vector for viewport
   */
  camera2d(vec2sz sz);

  /**
   * @brief Build using scalars for viewport
   *
   * @param w Viewport width
   * @param h Viewport height
   */
  camera2d(float w, float h);
  
public:
  /**
   * @brief Update view and projection matrices
   */
  void update();
  
public:
  /**
   * @brief Set camera viewport
   *
   * @param viewport Viewport size vector
   * @return this
   */
  inline camera2d& set_viewport(vec2 viewport);

  /**
   * @brief Set camera center
   *
   * @param center Camera center coordinates
   * @return this
   */
  inline camera2d& set_center(vec2 center);

  /**
   * @brief Set camera zoom
   *
   * @param zoom Zoom scalar
   * @return this
   */
  inline camera2d& set_zoom(float zoom);

  /**
   * @brief Set camera rotation (in radians)
   *
   * @param rot Rotation scalar
   * @return this
   */
  inline camera2d& set_rotation(float rot);

public:
  /**
   * @brief Projection matrix getter
   *
   * @return Projection matrix
   */
  mat4 proj() const { return _ccom.proj; }

  /**
   * @brief View matrix getter
   *
   * @return View matrix
   */
  mat4 view() const { return _ccom.view; }

  /**
   * @brief Center coordinates getter
   *
   * @return Center coordinates vec2
   */
  vec2 center() const { return _center; }

  /**
   * @brief Zoom scalar getter
   *
   * @return Zoom scalar
   */
  float zoom() const { return _zoom.x; }

  /**
   * @brief Rotation getter
   *
   * @return Rotation scalar (in radians)
   */
  float rot() const { return _rot; }

private:
  cam_common _ccom{};
  vec2 _origin{400.0f, 300.0f};
  vec2 _center{0.0f};
  vec2 _zoom{1.0f};
  float _rot{0.0f};
};

class camera3d {
public:
  /**
   * @brief Build using vector for viewport
   *
   * @param sz Size vector for viewport
   */
  camera3d(vec2sz sz);

  /**
   * @brief Build using scalars por viewport
   *
   * @param w Viewport width
   * @param h Viewport height
   */
  camera3d(float w, float h);

public:
  /**
   * @brief Update view and projection matrices
   */
  void update();

public:
  /**
   * @brief Set camera viewport
   *
   * @param viewport Viewport size vector
   * @return this
   */
  camera3d& set_viewport(vec2 viewport);

  /**
   * @brief Set camera position in world space
   *
   * @param pos World position vec3
   * @return this
   */
  camera3d& set_pos(vec3 pos);

  /**
   * @brief Set camera view direction
   *
   * @param dir View direction vec3 (normalized)
   * @return this
   */
  camera3d& set_dir(vec3 dir);

  /**
   * @brief Set zfar (draw distance)
   *
   * @param zfar Draw distance scalar
   * @return this
   */
  camera3d& set_zfar(float zfar);
  
  /**
   * @brief Set FOV
   *
   * @param fov FOV scalar
   * @return this
   */
  camera3d& set_fov(float fov);

  /**
   * @brief Use orthographic projection instead of perspective
   *
   * @param flag Enable boolean
   * @return this
   */
  camera3d& toggle_ortho(bool flag);

public:
  /**
   * @brief Projection matrix getter
   *
   * @return Projection matrix
   */
  mat4 proj() const { return _ccom.proj; }

  /**
   * @brief View matrix getter
   *
   * @return View matrix
   */
  mat4 view() const { return _ccom.view; }

  /**
   * @brief Position coordinates getter
   *
   * @return Position vec3
   */
  vec3 pos() const { return _pos; }

  /**
   * @brief View direction getter
   *
   * @return Direction vec3 (normalized)
   */
  vec3 dir() const { return _dir; }

  /**
   * @brief Check if camera is using orthographic projection
   *
   * @return Ortho flag boolean
   */
  bool ortho_flag() const { return _use_ortho; }

  /**
   * @brief FOV getter
   *
   * @return FOV scalar
   */
  float fov() const { return _fov; }

  /**
   * @brief zfar getter (draw distance)
   *
   * @return zfar scalar
   */
  float zfar() const { return _ccom.zfar; }

private:
  cam_common _ccom{};
  bool _use_ortho{false};
  vec3 _pos{0.0f};
  vec3 _dir{0.0f, 0.0f, -1.0f};
  vec3 _up{0.0f, 1.0f, 0.0f};
  float _fov{M_PIf*0.25f};
};

} // namespace ntf::shogle::scene

#ifndef CAMERA_INL_HPP
#include <shogle/scene/camera.inl.hpp>
#endif
