#pragma once

#include <shogle/math/alg.hpp>

#define SHOGLE_TRANSFORM_DECL_SETTER(signature) \
T& signature &;\
T&& signature &&

namespace ntf {

namespace impl {

template<std::size_t dim, typename T>
class transform;

template<typename T>
class transform<2, T> {
public:
  transform(vec2 pos = vec2{0.f}, vec2 scale = vec2{1.f}, vec3 rot = vec3{0.f}) :
    _pos(pos), _scale(scale), _rot(rot) {}

public:
  SHOGLE_TRANSFORM_DECL_SETTER(pos(vec2 pos));
  SHOGLE_TRANSFORM_DECL_SETTER(pos(float x, float y));
  SHOGLE_TRANSFORM_DECL_SETTER(pos(cmplx pos));
  SHOGLE_TRANSFORM_DECL_SETTER(pos_x(float x));
  SHOGLE_TRANSFORM_DECL_SETTER(pos_y(float y));

  SHOGLE_TRANSFORM_DECL_SETTER(scale(vec2 scale));
  SHOGLE_TRANSFORM_DECL_SETTER(scale(cmplx scale));
  SHOGLE_TRANSFORM_DECL_SETTER(scale(float scale));
  SHOGLE_TRANSFORM_DECL_SETTER(scale_x(float x));
  SHOGLE_TRANSFORM_DECL_SETTER(scale_y(float y));

  SHOGLE_TRANSFORM_DECL_SETTER(rot(float rot));
  SHOGLE_TRANSFORM_DECL_SETTER(rot(vec3 rot));
  SHOGLE_TRANSFORM_DECL_SETTER(rot(float euler_x, float euler_y, float euler_z));
  SHOGLE_TRANSFORM_DECL_SETTER(rot(vec2 euler_xy, float euler_z));
  SHOGLE_TRANSFORM_DECL_SETTER(rot(float euler_x, vec2 euler_yz));
  SHOGLE_TRANSFORM_DECL_SETTER(rot(cmplx euler_xy, float euler_z));
  SHOGLE_TRANSFORM_DECL_SETTER(rot(float euler_x, cmplx euler_yz));
  SHOGLE_TRANSFORM_DECL_SETTER(rot_x(float ang));
  SHOGLE_TRANSFORM_DECL_SETTER(rot_y(float ang));
  SHOGLE_TRANSFORM_DECL_SETTER(rot_z(float ang));

public:
  [[nodiscard]] vec2 pos() const { return _pos; }
  [[nodiscard]] vec2 scale() const { return _scale; }
  [[nodiscard]] float rot() const { return _rot.z; }

  [[nodiscard]] float pos_x() const { return _pos.x; }
  [[nodiscard]] float pos_y() const { return _pos.y;}
  [[nodiscard]] cmplx cpos() const { return cmplx{_pos.x, _pos.y}; }

  [[nodiscard]] float scale_x() const { return _scale.x; }
  [[nodiscard]] float scale_y() const { return _scale.y; }
  [[nodiscard]] cmplx cscale() const { return cmplx{_scale.x, _scale.y}; }

  [[nodiscard]] vec3 erot() const { return _rot; }
  [[nodiscard]] float rot_x() const { return _rot.x; }
  [[nodiscard]] float rot_y() const { return _rot.y; }
  [[nodiscard]] float rot_z() const { return _rot.z; }

  [[nodiscard]] bool dirty() const { return _dirty; }

public:
  [[nodiscard]] static mat4 build_matrix(vec2 pos, vec2 scale, float rot);
  [[nodiscard]] static mat4 build_matrix(vec2 pos, vec2 scale, vec3 rot);

protected:
  mat4 _mat;
  vec2 _pos;
  vec2 _scale;
  vec3 _rot;
  bool _dirty{true};
};


template<typename T>
class transform<3, T> {
public:
  transform(vec3 pos = vec3{0.f}, vec3 scale = vec3{1.f}, quat rot = quat{1.f, vec3{0.f}}) :
    _rot(rot), _pos(pos), _scale(scale) {};

public:
  SHOGLE_TRANSFORM_DECL_SETTER(pos(vec3 pos));
  SHOGLE_TRANSFORM_DECL_SETTER(pos(float x, float y, float z));
  SHOGLE_TRANSFORM_DECL_SETTER(pos(vec2 xy, float z));
  SHOGLE_TRANSFORM_DECL_SETTER(pos(float x, vec2 yz));
  SHOGLE_TRANSFORM_DECL_SETTER(pos(cmplx xy, float z));
  SHOGLE_TRANSFORM_DECL_SETTER(pos(float x, cmplx yz));
  SHOGLE_TRANSFORM_DECL_SETTER(pos_x(float x));
  SHOGLE_TRANSFORM_DECL_SETTER(pos_y(float y));
  SHOGLE_TRANSFORM_DECL_SETTER(pos_z(float z));

  SHOGLE_TRANSFORM_DECL_SETTER(scale(vec3 scale));
  SHOGLE_TRANSFORM_DECL_SETTER(scale(vec2 xy, float z));
  SHOGLE_TRANSFORM_DECL_SETTER(scale(float x, vec2 yz));
  SHOGLE_TRANSFORM_DECL_SETTER(scale(cmplx xy, float z));
  SHOGLE_TRANSFORM_DECL_SETTER(scale(float x, cmplx yz));
  SHOGLE_TRANSFORM_DECL_SETTER(scale(float scale));
  SHOGLE_TRANSFORM_DECL_SETTER(scale_x(float x));
  SHOGLE_TRANSFORM_DECL_SETTER(scale_y(float y));
  SHOGLE_TRANSFORM_DECL_SETTER(scale_z(float z));

  SHOGLE_TRANSFORM_DECL_SETTER(rot(quat rot));
  SHOGLE_TRANSFORM_DECL_SETTER(rot(float ang, vec3 axis));
  SHOGLE_TRANSFORM_DECL_SETTER(rot(vec3 euler_ang));
  SHOGLE_TRANSFORM_DECL_SETTER(rot(float euler_x, float euler_y, float euler_z));
  SHOGLE_TRANSFORM_DECL_SETTER(rot(vec2 euler_xy, float z));
  SHOGLE_TRANSFORM_DECL_SETTER(rot(float euler_x, vec2 euler_yz));
  SHOGLE_TRANSFORM_DECL_SETTER(rot(cmplx euler_xy, float z));
  SHOGLE_TRANSFORM_DECL_SETTER(rot(float euler_x, cmplx euler_yz));
  SHOGLE_TRANSFORM_DECL_SETTER(rot_x(float ang));
  SHOGLE_TRANSFORM_DECL_SETTER(rot_y(float ang));
  SHOGLE_TRANSFORM_DECL_SETTER(rot_z(float ang));

public:
  [[nodiscard]] vec3 pos() const { return _pos; }
  [[nodiscard]] vec3 scale() const { return _scale; }
  [[nodiscard]] quat rot() const { return _rot; }

  [[nodiscard]] float pos_x() const { return _pos.x; }
  [[nodiscard]] float pos_y() const { return _pos.y; }
  [[nodiscard]] float pos_z() const { return _pos.z; }

  [[nodiscard]] float scale_x() const { return _scale.x; }
  [[nodiscard]] float scale_y() const { return _scale.y; }
  [[nodiscard]] float scale_z() const { return _scale.z; }

  [[nodiscard]] vec3 erot() const { return glm::eulerAngles(_rot); }
  [[nodiscard]] float rot_x() const { return rot().x; }
  [[nodiscard]] float rot_y() const { return rot().y; }
  [[nodiscard]] float rot_z() const { return rot().z; }

  [[nodiscard]] bool dirty() const { return _dirty; }

public:
  [[nodiscard]] static mat4 build_matrix(vec3 pos, vec3 scale, quat rot);

protected:
  mat4 _mat;
  quat _rot;
  vec3 _pos;
  vec3 _scale;
  bool _dirty{true};
};

} // namespace impl

template<std::size_t dim>
class scene_graph : public impl::transform<dim, scene_graph<dim>> {
public:
  scene_graph() = default;

  template<typename... Args>
  explicit scene_graph(Args&&... args) :
    impl::transform<dim, scene_graph<dim>>{std::forward<Args>(args)...} {}

public:
  scene_graph& add_child(scene_graph* child) &;
  scene_graph&& add_child(scene_graph* child) &&;
  void force_update() &;

public:
  [[nodiscard]] const mat4& mat() &; // Not const

  [[nodiscard]] mat4 mat() &&; // Just build a matrix if it's an rvalue

private:
  scene_graph* _parent{nullptr};
  std::vector<scene_graph*> _children;

public:
  // TODO: Define move and copy things...
  ~scene_graph() noexcept;
  NTF_DISABLE_NOTHING(scene_graph);
};


template<std::size_t dim>
class transform : public impl::transform<dim, ::ntf::transform<dim>> {
public:
  transform() = default;

  template<typename... Args>
  explicit transform(Args&&... args) :
    impl::transform<dim, transform<dim>>{std::forward<Args>(args)...} {}

public:
  void force_update() &;

public:
  [[nodiscard]] const mat4& mat() &; // Not const

  [[nodiscard]] mat4 mat() &&; // Just build a matrix if it's an rvalue
};


using transform2d = transform<2>;
using transform3d = transform<3>;
using scene_graph2d = scene_graph<2>;
using scene_graph3d = scene_graph<3>;

} // namespace ntf

#undef SHOGLE_TRANSFORM_DECL_SETTER

#ifndef SHOGLE_TRANSFORM_INL
#include <shogle/scene/transform.inl>
#endif
