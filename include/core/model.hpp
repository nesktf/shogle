#pragma once

#include "core/game_object.hpp"
#include "core/texture.hpp"
#include <vector>

namespace ntf::shogle {

class Model : public GameObject {
public:
  class VAO {
  public:
    VAO();
    std::vector<Texture> tex;
    GLuint id(void) const { return vao_id; }
    size_t ind(void) const { return indices; }
  private:
    GLuint vao_id, vbo_id, ebo_id;
    size_t indices;
  };
  std::vector<VAO> vaos;
};

}
