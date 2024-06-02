#pragma once

#include <shogle/math/conversions.hpp>

#include <vector>

namespace ntf::shogle::scene {

template<typename T>
concept transf_dim = std::same_as<T, vec3> || std::same_as<T, vec2>;

/**
 * @brief Transform hierarchy implementation
 *
 * @tparam dim_t Dimension type
 * @tparam T Actual transform hierarchy type (for method chaining)
 */
template<transf_dim dim_t, typename T>
class transf_impl;

/**
 * @brief Transform hierarchy
 *
 * @tparam dim_t dimension type (vec2/vec3)
 */
template<transf_dim dim_t>
class transform : public transf_impl<dim_t, transform<dim_t>> {
public:
  transform();

public:
  /**
   * @brief Update transformation matrix if dirty (self or parent). Recursively updates children.
   *
   * @param parent Parent transform. If NULL treat this as root of the hierarchy.
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
   * @brief Transformation matrix getter
   *
   * @return Transformation matrix 
   */
  mat4 transf() const { return _mat; }

private:
  mat4 _mat {1.0f};

  std::vector<transform*> _children;
};

template<typename T>
class transf_impl<vec3, T> {
protected:
  transf_impl() = default;

public:
  /**
   * @brief Set position using vec3
   *
   * @param pos vec3 with coordinates
   * @return this
   */
  inline T& set_position(vec3 pos);

  /**
   * @brief Set position using scalars
   *
   * @param x X Coordinate
   * @param y Y Coordinate
   * @param z Z Coordinate
   * @return this
   */
  inline T& set_position(float x, float y, float z);

  /**
   * @brief Set scale using vec3
   *
   * @param scale Scale
   * @return this
   */
  inline T& set_scale(vec3 scale);

  /**
   * @brief Set scale using scalar. Same as set_scale(vec3{scale})
   *
   * @param scale Scale
   * @return this
   */
  inline T& set_scale(float scale);

  /**
   * @brief Set rotation using quaternion
   *
   * @param rot Rotation quaternion
   * @return this
   */
  inline T& set_rotation(quat rot);

  /**
   * @brief Set rotation using angle + vec3 axis
   *
   * @param ang Angle (in radians)
   * @param axis Rotation axis (normalized)
   * @return this
   */
  inline T& set_rotation(float ang, vec3 axis);

  /**
   * @brief Set rotation using vec3 with XYZ euler angles
   *
   * @param euler_ang XYZ euler angles (in radians)
   * @return this
   */
  inline T& set_rotation(vec3 euler_ang);

public:
  /**
   * @brief Position getter
   *
   * @return Position vec3
   */
  inline vec3 pos() const { return _pos; }

  /**
   * @brief Scale getter
   *
   * @return Scale vec3
   */
  inline vec3 scale() const { return _scale; }

  /**
   * @brief Rotation getter
   *
   * @return Rotation quaternion
   */
  inline quat rot() const { return _rot; }

  /**
   * @brief Rotation getter in euler angles
   *
   * @return Euler angles vec3 (in radians)
   */
  inline vec3 erot() const { return glm::eulerAngles(_rot); }

protected:
  bool _dirty {false};
  vec3 _pos {0.0f};
  vec3 _scale {1.0f};
  quat _rot {1.0f, vec3{0.0f}};
};

template<typename T>
class transf_impl<vec2, T> {
protected:
  transf_impl() = default;
  
public:
  /**
   * @brief Set position using vec2
   *
   * @param pos vec2 with coordinates
   * @return this
   */
  inline T& set_position(vec2 pos);

  /**
   * @brief Set position using scalars
   *
   * @param x X Coordinate
   * @param y Y Coordinate
   * @return this
   */
  inline T& set_position(float x, float y);

  /**
   * @brief Set position using complex value
   *
   * @param pos Complex coordinates
   * @return this
   */
  inline T& set_position(cmplx pos);

  /**
   * @brief Set scale using vec2
   *
   * @param scale Scale
   * @return this
   */
  inline T& set_scale(vec2 scale);

  /**
   * @brief Set scale using scalar. Same as set_scale(vec2{scale})
   *
   * @param scale Scale
   * @return this
   */
  inline T& set_scale(float scale);

  /**
   * @brief Set rotation angle
   *
   * @param rot Rotation angle (in radians)
   * @return this
   */
  inline T& set_rotation(float rot);

public:
  /**
   * @brief Position geter
   *
   * @return Position vec2
   */
  inline vec2 pos() const { return _pos; }

  /**
   * @brief Position getter in complex form
   *
   * @return Complex position
   */
  inline cmplx cpos() const { return cmplx{_pos.x, _pos.y}; }

  /**
   * @brief Scale getter
   *
   * @return Scale vec2
   */
  inline vec2 scale() const { return _scale; }

  /**
   * @brief Rotation angle getter
   *
   * @return Rotation angle (in radians)
   */
  inline float rot() const { return _rot; }

protected:
  bool _dirty {false};
  vec2 _pos {0.0f};
  vec2 _scale {1.0f};
  float _rot {0.0f};
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
  * @brief 2D transform hierarchy
  */
using transform2d = transform<vec2>;

/**
  * @brief 3D transform hierarchy
  */
using transform3d = transform<vec3>;

} // namespace ntf::shogle::scene

#ifndef TRANSFORM_INL_HPP
#include <shogle/scene/transform.inl.hpp>
#endif
