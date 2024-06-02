#pragma once

#include <shogle/core/types.hpp>

namespace ntf::shogle::math {

/**
 * @brief Convert OpenGL coordinates to regular cartesian coordinates
 *
 * @param v OpenGL coordinates
 * @return Cartesian coordinates
 */
inline vec3 ogl2car(vec3 v) { return {v.z, v.x, v.y}; }

/**
 * @brief Convert cartesian coordinates to OpenGL coordinates
 *
 * @param v Cartesian coordinates
 * @return OpenGL coordinates
 */
inline vec3 car2ogl(vec3 v) { return {v.y, v.z, v.x}; }

/**
 * @brief Convert spherical coordinates to cartesian coordinates
 *
 * @param rho
 * @param theta 
 * @param phi 
 * @return Cartesian coordinates
 */
inline vec3 sph2car(float rho, float theta, float phi) {
  return rho*vec3{glm::sin(theta)*glm::cos(phi), glm::sin(theta)*glm::sin(phi), glm::cos(theta)};
}

/**
 * @brief Convert spherical to OpenGL coordinates
 *
 * @param rho 
 * @param theta 
 * @param phi 
 * @return OpenGL coordinates
 */
inline vec3 sph2ogl(float rho, float theta, float phi) {
  return car2ogl(sph2car(rho, theta, phi));
}

/**
 * @brief Build rotation quaternion from normalized axis and angle scalar
 *
 * @param ang Angle
 * @param axis Axis (normalized)
 * @return Rotation quaternion
 */
inline quat axisquat(float ang, vec3 axis) {
  return quat{glm::cos(ang*0.5f), glm::sin(ang*0.5f)*axis};
}

/**
 * @brief Convert rotation quaternion to euler coordinates
 *
 * @param rot Rotation quaternion
 * @return Euler coordinates vec3
 */
inline quat eulerquat(vec3 rot) {
  return
    axisquat(rot.x, vec3{1.0f, 0.0f, 0.0f}) *
    axisquat(rot.y, vec3{0.0f, 1.0f, 0.0f}) *
    axisquat(rot.z, vec3{0.0f, 0.0f, 1.0f});
};

/**
 * @brief exp(i*theta) shorthand for vectors
 *
 * @param theta 
 * @return vec2
 */
inline vec2 expiv(float theta) {
  return vec2{glm::cos(theta), glm::sin(theta)};
}

/**
 * @brief exp(i*theta) shorthand for complex
 *
 * @param theta 
 * @return cmplx
 */
inline cmplx expic(float theta) {
  return cmplx{glm::cos(theta), glm::sin(theta)};
}

/**
 * @brief Convert degrees to radians
 *
 * @param deg Degrees
 * @return Radians
 */
inline float rad(float deg) {
  return glm::radians(deg);
}

/**
 * @brief Convert radians to degrees
 *
 * @param rad Radians
 * @return Degrees
 */
inline float deg(float rad) {
  return glm::degrees(rad);
}

} // namespace ntf::shogle::math
