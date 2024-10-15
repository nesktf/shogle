#define SHOGLE_TRANSFORM_INL
#include <shogle/scene/transform.hpp>
#undef SHOGLE_TRANSFORM_INL

#define SHOGLE_TRANSFORM_DEF_SETTER(dim, signature, ...) \
template<typename T> \
T& impl::transform<dim, T>::signature & { \
  __VA_ARGS__ \
  _dirty = true; \
  return static_cast<T&>(*this); \
} \
template<typename T> \
T&& impl::transform<dim, T>::signature && { \
  __VA_ARGS__ \
  _dirty = true; \
  return static_cast<T&&>(*this); \
}

namespace ntf {

SHOGLE_TRANSFORM_DEF_SETTER(2, pos(vec2 pos),
  _pos = pos;
)

SHOGLE_TRANSFORM_DEF_SETTER(2, pos(float x, float y),
  _pos = vec2{x, y};
)

SHOGLE_TRANSFORM_DEF_SETTER(2, pos(cmplx pos),
  _pos = vec2{pos.real(), pos.imag()};
)

SHOGLE_TRANSFORM_DEF_SETTER(2, pos_x(float x),
  _pos.x = x;
)

SHOGLE_TRANSFORM_DEF_SETTER(2, pos_y(float y),
  _pos.y = y;
)

SHOGLE_TRANSFORM_DEF_SETTER(2, scale(vec2 scale),
  _scale = scale;
)

SHOGLE_TRANSFORM_DEF_SETTER(2, scale(cmplx scale),
  _scale = vec2{scale.real(), scale.imag()};
)

SHOGLE_TRANSFORM_DEF_SETTER(2, scale(float scale),
  _scale = vec2{scale, scale};
)

SHOGLE_TRANSFORM_DEF_SETTER(2, scale_x(float x),
  _scale.x = x;
)

SHOGLE_TRANSFORM_DEF_SETTER(2, scale_y(float y),
  _scale.y = y;
)

SHOGLE_TRANSFORM_DEF_SETTER(2, rot(float rot),
  _rot.z = rot;
)

SHOGLE_TRANSFORM_DEF_SETTER(2, rot(vec3 rot),
  _rot = rot;
)

SHOGLE_TRANSFORM_DEF_SETTER(2, rot(float euler_x, float euler_y, float euler_z),
  _rot = vec3{euler_x, euler_y, euler_z};
)

SHOGLE_TRANSFORM_DEF_SETTER(2, rot(vec2 euler_xy, float euler_z),
  _rot = vec3{euler_xy.x, euler_xy.y, euler_z};
)

SHOGLE_TRANSFORM_DEF_SETTER(2, rot(float euler_x, vec2 euler_yz),
  _rot = vec3{euler_x, euler_yz.x, euler_yz.y};
)

SHOGLE_TRANSFORM_DEF_SETTER(2, rot(cmplx euler_xy, float euler_z),
  _rot = vec3{euler_xy.real(), euler_xy.imag(), euler_z};
)

SHOGLE_TRANSFORM_DEF_SETTER(2, rot(float euler_x, cmplx euler_yz),
  _rot = vec3{euler_x, euler_yz.real(), euler_yz.imag()};
)

SHOGLE_TRANSFORM_DEF_SETTER(2, rot_x(float ang),
  _rot.x = ang;
)

SHOGLE_TRANSFORM_DEF_SETTER(2, rot_y(float ang),
  _rot.y = ang;
)

SHOGLE_TRANSFORM_DEF_SETTER(2, rot_z(float ang),
  _rot.z = ang;
)

template<typename T>
mat4 impl::transform<2, T>::build_matrix(vec2 pos, vec2 scale, float rot) {
  mat4 model{1.0f};

  model = glm::translate(model, vec3{pos, 0.0f});
  model = glm::rotate(model, rot, vec3{0.0f, 0.0f, 1.0f});
  model = glm::scale(model, vec3{scale, 1.0f});

  return model;
}

template<typename T>
mat4 impl::transform<2, T>::build_matrix(vec2 pos, vec2 scale, vec3 rot) {
  mat4 model{1.0f};

  model = glm::translate(model, vec3{pos, 0.0f});
  model = glm::rotate(model, rot.x, vec3{1.0f, 0.0f, 0.0f});
  model = glm::rotate(model, rot.y, vec3{0.0f, 1.0f, 0.0f});
  model = glm::rotate(model, rot.z, vec3{0.0f, 0.0f, 1.0f});
  model = glm::scale(model, vec3{scale, 1.0f});

  return model;
}


SHOGLE_TRANSFORM_DEF_SETTER(3, pos(vec3 pos),
  _pos = pos;
)

SHOGLE_TRANSFORM_DEF_SETTER(3, pos(float x, float y, float z),
  _pos = vec3{x, y, z};
)

SHOGLE_TRANSFORM_DEF_SETTER(3, pos(vec2 xy, float z),
  _pos = vec3{xy.x, xy.y, z};
)

SHOGLE_TRANSFORM_DEF_SETTER(3, pos(float x, vec2 yz),
  _pos = vec3{x, yz.x, yz.y};
)

SHOGLE_TRANSFORM_DEF_SETTER(3, pos(cmplx xy, float z),
  _pos = vec3{xy.real(), xy.imag(), z};
)

SHOGLE_TRANSFORM_DEF_SETTER(3, pos(float x, cmplx yz),
  _pos = vec3{x, yz.real(), yz.imag()};
)

SHOGLE_TRANSFORM_DEF_SETTER(3, pos_x(float x),
  _pos.x = x;
)

SHOGLE_TRANSFORM_DEF_SETTER(3, pos_y(float y),
  _pos.y = y;
)

SHOGLE_TRANSFORM_DEF_SETTER(3, pos_z(float z),
  _pos.z = z;
)

SHOGLE_TRANSFORM_DEF_SETTER(3, scale(vec3 scale),
  _scale = scale;
)

SHOGLE_TRANSFORM_DEF_SETTER(3, scale(vec2 xy, float z),
  _scale = vec3{xy.x, xy.y, z};
)

SHOGLE_TRANSFORM_DEF_SETTER(3, scale(float x, vec2 yz),
  _scale = vec3{x, yz.x, yz.y};
)

SHOGLE_TRANSFORM_DEF_SETTER(3, scale(cmplx xy, float z),
  _scale = vec3{xy.real(), xy.imag(), z};
)

SHOGLE_TRANSFORM_DEF_SETTER(3, scale(float x, cmplx yz),
  _scale = vec3{x, yz.real(), yz.imag()};
)

SHOGLE_TRANSFORM_DEF_SETTER(3, scale(float scale),
  _scale = vec3{scale, scale, scale};
)

SHOGLE_TRANSFORM_DEF_SETTER(3, scale_x(float x),
  _scale.x = x;
)

SHOGLE_TRANSFORM_DEF_SETTER(3, scale_y(float y),
  _scale.y = y;
)

SHOGLE_TRANSFORM_DEF_SETTER(3, scale_z(float z),
  _scale.z = z;
)

SHOGLE_TRANSFORM_DEF_SETTER(3, rot(quat rot),
  _rot = rot;
)

SHOGLE_TRANSFORM_DEF_SETTER(3, rot(float ang, vec3 axis),
  _rot = ::ntf::axisquat(ang, axis);
)

SHOGLE_TRANSFORM_DEF_SETTER(3, rot(vec3 euler_ang),
  _rot = ::ntf::eulerquat(euler_ang);
)

SHOGLE_TRANSFORM_DEF_SETTER(3, rot(float euler_x, float euler_y, float euler_z),
  _rot = ::ntf::eulerquat(vec3{euler_x, euler_y, euler_z});
)

SHOGLE_TRANSFORM_DEF_SETTER(3, rot(vec2 euler_xy, float euler_z),
  _rot = ::ntf::eulerquat(vec3{euler_xy.x, euler_xy.y, euler_z});
)

SHOGLE_TRANSFORM_DEF_SETTER(3, rot(float euler_x, vec2 euler_yz),
  _rot = ::ntf::eulerquat(vec3{euler_x, euler_yz.x, euler_yz.y});
)

SHOGLE_TRANSFORM_DEF_SETTER(3, rot(cmplx euler_xy, float euler_z),
  _rot = ::ntf::eulerquat(vec3{euler_xy.real(), euler_xy.imag(), euler_z});
)

SHOGLE_TRANSFORM_DEF_SETTER(3, rot(float euler_x, cmplx euler_yz),
  _rot = ::ntf::eulerquat(vec3{euler_x, euler_yz.real(), euler_yz.imag()});
)

SHOGLE_TRANSFORM_DEF_SETTER(3, rot_x(float ang),
  _rot.x = ang;
)

SHOGLE_TRANSFORM_DEF_SETTER(3, rot_y(float ang),
  _rot.y = ang;
)

SHOGLE_TRANSFORM_DEF_SETTER(3, rot_z(float ang),
  _rot.z = ang;
)

template<typename T>
mat4 impl::transform<3, T>::build_matrix(vec3 pos, vec3 scale, quat rot) {
  mat4 model{1.0f};

  model = glm::translate(model, pos);
  model*= glm::mat4_cast(rot);
  model = glm::scale(model, scale);

  return model;
}


template<std::size_t dim>
scene_graph<dim>::~scene_graph() noexcept {
  for (auto* child : _children) {
    child->_parent = nullptr;
  }
}

template<std::size_t dim>
auto scene_graph<dim>::add_child(scene_graph* child) & -> scene_graph& {
  _children.push_back(child);
  child->_parent = this;
  child->_dirty = true; // force child to update
  return *this;
}

template<std::size_t dim>
auto scene_graph<dim>::add_child(scene_graph* child) && -> scene_graph&& {
  _children.push_back(child);
  child->_parent = this;
  child->_dirty = true; // force child to update
  return std::move(*this);
}

template<std::size_t dim>
void scene_graph<dim>::force_update() & {
  if (_parent) {
    this->_mat = _parent->mat()*this->build_matrix(this->_pos, this->_scale, this->_rot);
  } else {
    this->_mat = this->build_matrix(this->_pos, this->_scale, this->_rot);
  }
  for (auto* child : _children) {
    child->_dirty = true; // child may update later
  }
  this->_dirty = false;
}

template<std::size_t dim>
const mat4& scene_graph<dim>::mat() & {
  if (this->_dirty) {
    force_update();
  }
  return this->_mat;
}

template<std::size_t dim>
mat4 scene_graph<dim>::mat() && {
  // TODO: Test this?
  if (_parent) {
    return _parent->mat()*this->build_matrix(this->_pos, this->_scale, this->_rot);
  }
  return this->build_matrix(this->_pos, this->_scale, this->_rot);
}


template<std::size_t dim>
void transform<dim>::force_update() & {
  this->_mat = this->build_matrix(this->_pos, this->_scale, this->_rot);
  this->_dirty = false;
}

template<std::size_t dim>
const mat4& transform<dim>::mat() & {
  if (this->_dirty) {
    force_update();
  }
  return this->_mat;
}

template<std::size_t dim>
mat4 transform<dim>::mat() && {
  return this->build_matrix(this->_pos, this->_scale, this->_rot);
}

} // namespace ntf

#undef SHOGLE_TRANSFORM_DEF_SETTER
