#pragma once

#include "render/drawable.hpp"
#include <glm/gtx/transform.hpp>

namespace ntf::shogle {

class GameObject {
public:
  virtual ~GameObject() = default;

public:
  virtual void update(float dt) = 0;

protected:
  static glm::mat4 gen_model_3d(glm::vec3 pos, glm::vec3 scale, glm::vec3 rot) {
    glm::mat4 mat{1.0f}, rot_mat{1.0f};

    mat = glm::translate(mat, pos);
    mat = glm::scale(mat, scale);
    rot_mat = glm::rotate(rot_mat, glm::radians(rot.x), glm::vec3{1.0f, 0.0f, 0.0f});
    rot_mat = glm::rotate(rot_mat, glm::radians(rot.y), glm::vec3{0.0f, 1.0f, 0.0f});
    rot_mat = glm::rotate(rot_mat, glm::radians(rot.z), glm::vec3{0.0f, 0.0f, 1.0f});

    return mat * rot_mat;
  }

  static glm::mat4 gen_model_2d(glm::vec2 pos, glm::vec2 scale, float rot) {
    glm::mat4 mat{1.0f}, rot_mat{1.0f};

    mat = glm::translate(mat, glm::vec3{pos, 0.0f});
    mat = glm::scale(mat, glm::vec3{scale, 1.0f});
    rot_mat = glm::rotate(rot_mat, glm::radians(rot), glm::vec3{0.0f, 0.0f, 1.0f});

    return mat * rot_mat;
  }

public:
  bool enabled = true;
  Drawable* obj;
};

} // namespace ntf::shogle
