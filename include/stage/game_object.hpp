#pragma once

#include "core/shader.hpp"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace ntf::shogle {

class GameObject {
public:
  GameObject(Shader& _shader) : shader(std::ref(_shader)) {}
  virtual ~GameObject() = default;

public:
  virtual void update(float dt) = 0;
  virtual void draw(void) const = 0;

  virtual void set_pos(const glm::vec3& new_pos) = 0;
  virtual void set_scale(const glm::vec3& new_scale) = 0;
  virtual void set_rot(const glm::vec3& new_rot) = 0;
  virtual void set_rot(float new_rot) = 0;

public:
  std::reference_wrapper<Shader> shader;
  glm::mat4 model_m;
  glm::vec3 pos_v, scale_v, rot_v;
};

} // namespace ntf::shogle
