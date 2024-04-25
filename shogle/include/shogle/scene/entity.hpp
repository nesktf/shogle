#pragma once

#include <shogle/core/types.hpp>
#include <shogle/scene/scene.hpp>

namespace ntf {

template<typename dim_t>
requires(std::same_as<dim_t, vec2> || std::same_as<dim_t, vec3>)
class Entity : public Scene::Object {
protected:
  Entity() = default;

public:
  virtual void update(float dt) override {
    if (_update_transform_flag) {
      update_transform(dt);
      _update_transform_flag = false;
    }
  }

public:
  inline Entity& set_scale(dim_t scale) {
    _scale = scale;
    _update_transform_flag = true;
    return *this;
  }

  inline Entity& set_pos(vec3 pos) {
    _pos = pos;
    _update_transform_flag = true;
    return *this;
  }

  template<typename _dim_t = dim_t>
  requires(std::same_as<_dim_t, vec2>)
  inline Entity& set_pos(vec2 pos, float layer = 0.0f) {
    _pos = vec3{pos, layer};
    _update_transform_flag = true;
    return *this;
  }

  inline Entity& set_vel(vec3 vel) {
    _vel = vel;
    _update_transform_flag = true;
    return *this;
  }

  template<typename _dim_t = dim_t>
  requires(std::same_as<_dim_t, vec2>)
  inline Entity& set_vel(vec2 vel) {
    _vel = vec3{vel, 0.0f};
    _update_transform_flag = true;
    return *this;
  }

  inline Entity& set_rot(quat rot) {
    _rot = rot;
    _update_transform_flag = true;
    return *this;
  }

  template<typename _dim_t = dim_t>
  requires(std::same_as<_dim_t, vec3>)
  inline Entity& set_rot(float rot, vec3 axis) {
    return set_rot(quat{glm::cos(rot*0.5f), glm::sin(rot*0.5f)*axis});
  }

  template<typename _dim_t = dim_t>
  requires(std::same_as<_dim_t, vec2>)
  inline Entity& set_rot(float rot) {
    vec3 axis_2d {0.0f, 0.0f, 1.0f};
    return set_rot(quat{glm::cos(rot*0.5f), glm::sin(rot*0.5f)*axis_2d});
  }

public:
  inline float rot_ang(void) const { return glm::acos(_rot.x)*2.0f; }
  inline quat rot(void) const { return _rot; }
  inline dim_t pos(void) const { return _pos; }
  inline dim_t scale(void) const { return _scale; }

protected:
  Entity& update_transform(float dt) = 0;

private:
  vec3 _pos {0.0f}, _vel {0.0f};
  dim_t _scale {1.0f};
  quat _rot {1.0f, vec3{0.0f}};

  bool _update_transform_flag {false};
};

using Entity2D = Entity<vec2>;
using Entity3D = Entity<vec3>;

} // namespace ntf
