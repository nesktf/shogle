#pragma once

#include "glad/glad.h"
#include "core/shader.hpp"
#include "resources/texture_data.hpp"
#include <assimp/texture.h>

namespace ntf::shogle {

class Texture {
public:
  Texture(const TextureData* data);
  ~Texture();

  void bind_material(Shader& shader, size_t tex_num, size_t tex_ind);
  float aspect(void) const { return (float)width/(float)height;}
  GLuint id(void) const { return tex; }
  aiTextureType type(void) const { return tex_type; }

private:
  int width, height;

  GLuint tex;
  GLenum dim;
  aiTextureType tex_type;
};

} 
