#pragma once

#include <shogle/core/types.hpp>

#include <shogle/scene/scene.hpp>
#include <shogle/scene/util.hpp>

namespace ntf {

template<typename dim_t>
requires(std::same_as<dim_t, vec2> || std::same_as<dim_t, vec3>)
class entity : public scene::drawable {
protected:
  entity() = default;

public:
  virtual void update(float) override {
    mat4 model {1.0f};

    if constexpr (std::same_as<dim_t, vec3>) {
      model = glm::translate(model, _pos);
      model*= glm::mat4_cast(_rot);
      model = glm::scale(model, _scale);
    } else {
      model = glm::translate(model, vec3{_pos, 0.0f});
      model*= glm::mat4_cast(_rot);
      model = glm::scale(model, vec3{_scale, 1.0f});
    }

    _model_mat = model;
  }
public:
  inline mat4 model_mat(void) { return _model_mat; }

public:

  static inline void rotate(entity& obj, quat rot) {
    obj._rot = rot;
  }

  static inline void move(entity& obj, dim_t pos) {
    obj._pos = pos;
  }

  static inline void scale(entity& obj, dim_t scale) {
    obj._scale = scale;
  }

  static inline void toggle_screen_space(entity& obj, bool flag = true) {
    obj._use_screen_space = flag;
  }

  template<typename _dim_t = dim_t>
  requires(std::same_as<_dim_t, vec3>)
  static inline void rotate(entity& obj, float ang, vec3 axis) {
    obj._rot = quat{glm::cos(ang*0.5f), glm::sin(ang*0.5f)*axis};
  }

  template<typename _dim_t = dim_t>
  requires(std::same_as<_dim_t, vec3>)
  static inline void rotate(entity& obj, vec3 euler_rot) {
    obj._rot = euler2quat(euler_rot);
  }

  template<typename _dim_t = dim_t>
  requires(std::same_as<_dim_t, vec2>)
  static inline void rotate(entity& obj, float ang) {
    obj._rot = quat{glm::cos(-ang*0.5f), vec3{0.0f, 0.0f, glm::sin(-ang*0.5f)}};
  }

public:
  bool _use_screen_space {false};
  dim_t _pos {0.0f};
  dim_t _scale {1.0f};
  quat _rot {1.0f, vec3{0.0f}};

private:
  mat4 _model_mat {1.0f};
};

using entity2d = entity<vec2>;
using entity3d = entity<vec3>;

} // namespace ntf
