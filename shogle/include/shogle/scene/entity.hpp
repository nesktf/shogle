#pragma once

#include <shogle/core/types.hpp>
#include <shogle/scene/scene.hpp>

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
  inline mat4 model_mat(void) { return _model_mat; }

public:
  dim_t pos {0.0f};
  dim_t scale {1.0f};
  quat rot {1.0f, vec3{0.0f}};

private:
  mat4 _model_mat {1.0f};
};

using entity2D = entity<vec2>;
using entity3D = entity<vec3>;

} // namespace ntf
