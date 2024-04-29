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
  virtual void update(float) override {
    mat4 model {1.0f};

    if constexpr (std::same_as<dim_t, vec3>) {
      model = glm::translate(model, pos);
      model*= glm::mat4_cast(rot);
      model = glm::scale(model, scale);
    } else {
      model = glm::translate(model, vec3{pos, 0.0f});
      model*= glm::mat4_cast(rot);
      model = glm::scale(model, vec3{scale, 1.0f});
    }

    _model_mat = model;
  }

public:
  inline mat4 model(void) { return _model_mat; }

public:
  dim_t pos {0.0f};
  dim_t scale {1.0f};
  quat rot {1.0f, vec3{0.0f}};

private:
  mat4 _model_mat {1.0f};
};

using Entity2D = Entity<vec2>;
using Entity3D = Entity<vec3>;

template<typename T>
inline void rotate_entity(T& obj, float ang, vec3 axis) {
  obj.rot = quat{glm::cos(ang*0.5f), glm::sin(ang*0.5f)*axis};
}
inline void rotate_entity(Entity2D& obj, float ang) {
  rotate_entity(obj, -ang, vec3{0.0f, 0.0f, 1.0f});
}

template<typename T, typename dim_t>
inline void move_entity(T& obj, dim_t pos) {
  obj.pos = pos;
}

template<typename T, typename dim_t>
inline void scale_entity(T& obj, dim_t scale) {
  obj.scale = scale;
}

} // namespace ntf
