#pragma once

#include "glad/glad.h"
#include "core/shader.hpp"
#include "util/singleton.hpp"
#include "core/model.hpp"
#include "core/sprite.hpp"

namespace ntf::shogle {

class Renderer : public Singleton<Renderer> {
public:
  Renderer();
  ~Renderer();

  void draw(Shader& shader, Model& model) const;
  void draw(Shader& shader, Sprite& sprite) const;

  void update_proj_m(size_t width, size_t height);
  void update_view_m(float yaw, float pitch);
  void update_view_m(const glm::vec3& vec);

private:
  GLuint q_VAO, q_VBO, q_EBO;
  glm::mat4 oproj_m, pproj_m, view_m;

public:
  glm::vec3 view_pos, up_vec, dir_vec;
  float cam_speed, cam_sens, fov;
};

}
