#include <shogle/scene/transform.hpp>

namespace ntf::shogle {

mat4 transf_mat(vec3 pos, vec3 scale, quat rot) {
  mat4 model{1.0f};

  model = glm::translate(model, pos);
  model*= glm::mat4_cast(rot);
  model = glm::scale(model, scale);

  return model;
}

mat4 transf_mat(vec2 pos, vec2 scale, float rot) {
  mat4 model{1.0f};

  model = glm::translate(model, vec3{pos, 0.0f});
  model = glm::rotate(model, rot, vec3{0.0f, 0.0f, 1.0f});
  model = glm::scale(model, vec3{scale, 1.0f});

  return model;
}

} // namespace ntf::shogle
