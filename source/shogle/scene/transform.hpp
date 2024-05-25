#pragma once

#include <shogle/math/conversions.hpp>

#include <vector>

namespace ntf::shogle::scene {

/**
 * @brief Object transform with hierarchy
 *
 * @tparam dim_t dimension type (vec2/vec3)
 * @tparam rot_t rotation type (float/quat)
 */
template<typename dim_t, typename rot_t>
class transform {
public:
  static constexpr size_t dim_size = std::same_as<dim_t, vec3> ? 3 : 2;

public:
  transform();

public:
  /**
   * @brief Update transformation matrix if dirty (self or parent). Recursively updates children.
   *
   * @param parent Parent transform. If NULL treat as parent of the hierarchy.
   */
  void update(const transform* parent = nullptr);

  /**
   * @brief Add child to the transform hierarchy
   *
   * @param child Child pointer
   */
  void add_child(transform* child);

public:
  /**
   * @brief Position setter
   *
   * @param pos Position vector
   * @return this
   */
  inline transform& set_position(dim_t pos);

  /**
   * @brief Set position with complex number
   *
   * @param pos Complex position
   * @return this
   */
  inline transform& set_position(cmplx pos) requires(dim_size == 2);

  /**
   * @brief Set position with coordinates
   *
   * @param x Position x coord
   * @param y Position y coord
   * @return this
   */
  inline transform& set_position(float x, float y) requires(dim_size == 2);

  /**
   * @brief Set position with coordinates
   *
   * @param x Position x coord
   * @param y Position y coord
   * @param z Position z coord
   * @return this
   */
  inline transform& set_position(float x, float y, float z) requires(dim_size == 3);

  /**
   * @brief Scale setter
   *
   * @param scale Scale vector
   * @return this
   */
  inline transform& set_scale(dim_t scale);

  /**
   * @brief Set scale on all coordinates. Same as set_scale(vec{scale})
   *
   * @param scale Scale scalar
   * @return this
   */
  inline transform& set_scale(float scale);

  /**
   * @brief Set rotation directly
   *
   * @param rot Rotation
   * @return this
   */
  inline transform& set_rotation(rot_t rot);

  /**
   * @brief Set rotation quaternion from axis + angle
   *
   * @param ang Angle
   * @param axis Axis (normalized)
   * @return this
   */
  inline transform& set_rotation(float ang, vec3 axis) requires(dim_size == 3);

  /**
   * @brief Set rotation quaternion from euler angles
   *
   * @param euler_ang Angles
   * @return this
   */
  inline transform& set_rotation(vec3 euler_ang) requires(dim_size == 3);

public:
  /**
   * @brief Transformation matrix getter
   *
   * @return Transformation matrix 
   */
  mat4 transf() const { return _mat; }

  /**
   * @brief Position getter
   *
   * @return Position vector
   */
  dim_t pos() const { return _pos; }

  /**
   * @brief Scale getter
   *
   * @return Scale vector
   */
  dim_t scale() const { return _scale; }

  /**
   * @brief Rotation getter
   *
   * @return Rotation
   */
  rot_t rot() const { return _rot; }

  /**
   * @brief Complex position getter
   *
   * @return Complex position
   */
  cmplx cpos() const requires(dim_size == 2) { return cmplx{_pos.x, _pos.y}; }

  /**
   * @brief Euler angles getter
   *
   * @return Euler angles vec3
   */
  vec3 erot() const requires(dim_size == 3) { return glm::eulerAngles(_rot); }

private:
  mat4 _mat {1.0f};
  bool _dirty {false};

  dim_t _pos {0.0f};
  dim_t _scale {1.0f};
  rot_t _rot {};

  std::vector<transform*> _children;
};


/**
 * @brief 3D model matrix generator
 *
 * @param pos Position vec3
 * @param scale Scale vec3
 * @param rot Rotation quaternion
 * @return Transformation matrix
 */
mat4 transf_mat(vec3 pos, vec3 scale, quat rot);

/**
 * @brief 2D model matrix generator
 *
 * @param pos Position vec2
 * @param scale Scale vec2 
 * @param rot Rotation scalar
 * @return Transformation matrix
 */
mat4 transf_mat(vec2 pos, vec2 scale, float rot);


/**
  * @brief 2D Object transform with hierarchy
  */
using transform2d = transform<vec2, float>;

/**
  * @brief 3D Object transform with hierarchy
  */
using transform3d = transform<vec3, quat>;

} // namespace ntf::shogle::scene

#ifndef TRANSFORM_INL_HPP
#include <shogle/scene/transform.inl.hpp>
#endif
