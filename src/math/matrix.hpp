#pragma once

#include "./vector.hpp"

#if SHOGLE_USE_GLM
#include <glm/gtc/matrix_transform.hpp>
#endif

namespace ntf {

#if SHOGLE_USE_GLM
template<uint32 N, uint32 M, typename T>
using mat = glm::mat<N, M, T>;
#endif

using mat3 = mat<3, 3, float32>;
using mat4 = mat<4, 4, float32>;

using dmat3 = mat<3, 3, float64>;
using dmat4 = mat<4, 4, float64>;

template<typename T>
mat<4, 4, T> build_trs_matrix(const vec<2, T>& pos,
                              const vec<2, T>& scale,
                              const vec<3, T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, vec<3, T>{pos, T{0}});
  m = glm::rotate(m, rot.x, vec<3, T>{T{1}, T{0}, T{0}});
  m = glm::rotate(m, rot.y, vec<3, T>{T{0}, T{1}, T{0}});
  m = glm::rotate(m, rot.z, vec<3, T>{T{0}, T{0}, T{1}});
  m = glm::scale(m, vec<3, T>{scale, T{1}});
  return m;
}

template<typename T>
mat<4, 4, T> build_trs_matrix(const vec<3, T>& pos,
                              const vec<3, T>& scale,
                              const vec<3, T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, pos);
  m = glm::rotate(m, rot.x, vec<3, T>{T{1}, T{0}, T{0}});
  m = glm::rotate(m, rot.y, vec<3, T>{T{0}, T{1}, T{0}});
  m = glm::rotate(m, rot.z, vec<3, T>{T{0}, T{0}, T{1}});
  m = glm::scale(m, scale);
  return m;
}

template<typename T>
mat<4, 4, T> build_trs_matrix(const vec<2, T>& pos,
                              const vec<2, T>& scale,
                              const vec<2, T>& pivot,
                              const vec<3, T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, vec<3, T>{pos.x, pos.y, T{0}});
  m = glm::translate(m, vec<3, T>{pivot.x, pivot.y, T{0}});
  m = glm::rotate(m, rot.x, vec<3, T>{T{1}, T{0}, T{0}});
  m = glm::rotate(m, rot.y, vec<3, T>{T{0}, T{1}, T{0}});
  m = glm::rotate(m, rot.z, vec<3, T>{T{0}, T{0}, T{1}});
  m = glm::translate(m, vec<3, T>{-pivot.x, -pivot.y, T{0}});
  m = glm::scale(m, vec<3, T>{scale.x, scale.y, T{1}});
  return m;
}

template<typename T>
mat<4, 4, T> build_trs_matrix(const vec<3, T>& pos,
                              const vec<3, T>& scale,
                              const vec<3, T>& pivot,
                              const vec<3, T>& rot) noexcept {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, pos);
  m = glm::translate(m, pivot);
  m = glm::rotate(m, rot.x, vec<3, T>{T{1}, T{0}, T{0}});
  m = glm::rotate(m, rot.y, vec<3, T>{T{0}, T{1}, T{0}});
  m = glm::rotate(m, rot.z, vec<3, T>{T{0}, T{0}, T{1}});
  m = glm::translate(m, -pivot);
  m = glm::scale(m, scale);
  return m;
}

template<typename T>
mat<4, 4, T> build_view_matrix(const vec<2, T>& pos,
                               const vec<2, T>& pivot,
                               const vec<2, T>& zoom,
                               const vec<3, T>& rot) {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, vec<3, T>{pivot.x, pivot.y, T{0}});
  m = glm::rotate(m, rot.x, vec<3, T>{T{1}, T{0}, T{0}});
  m = glm::rotate(m, rot.y, vec<3, T>{T{0}, T{1}, T{0}});
  m = glm::rotate(m, rot.z, vec<3, T>{T{0}, T{0}, T{1}});
  m = glm::scale(m, vec<3, T>{zoom.x, zoom.y, T{1}});
  m = glm::translate(m, vec<3, T>{-pos.x, -pos.y, T{0}});
  return m;
}

template<typename T>
mat<4, 4, T> build_view_matrix(const vec<2, T>& pos,
                               const vec<2, T>& pivot,
                               const vec<2, T>& zoom,
                               const T& rot,
                               const vec<3, T>& axis) {
  mat<4, 4, T> m{T{1}};
  m = glm::translate(m, vec<3, T>{pivot.x, pivot.y, T{0}});
  m = glm::rotate(m, rot, axis);
  m = glm::scale(m, vec<3, T>{zoom.x, zoom.y, T{1}});
  m = glm::translate(m, vec<3, T>{-pos.x, -pos.y, T{0}});
  return m;
}

template<typename T>
mat<4, 4, T> build_view_matrix(const vec<3, T>& pos,
                               const vec<3, T>& dir,
                               const vec<3, T>& up) {
  return glm::lookAt(pos, pos+dir, up);
}

template<typename T>
mat<4, 4, T> build_proj_matrix(const vec<2, T>& viewport,
                               const T& znear,
                               const T& zfar) {
  return glm::ortho(T{0}, viewport.x,
                    viewport.y, T{0},
                    znear, zfar);
}

template<typename T>
mat<4, 4, T> build_proj_matrix(const vec<2, T>& viewport,
                               const T& znear,
                               const T& zfar,
                               const T& fov) {
  return glm::perspective(fov, viewport.x/viewport.y, znear, zfar);
}

} // namespace ntf
