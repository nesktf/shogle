#pragma once

#include "glad/glad.h"
#include "core/shader.hpp"
#include "util/singleton.hpp"

namespace ntf::shogle {

class Renderer : public Singleton<Renderer> {
public:
  Renderer();

  template<typename T>
  void draw(Shader& shader, T& obj);
private:
  GLuint q_VAO, q_VBO, q_EBO;
  glm::mat4 oproj_m, pproj_m, view_m;
  glm::vec3 view_pos;
};

}
