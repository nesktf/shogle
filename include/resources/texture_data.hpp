#pragma once

#include "glad/glad.h"

#include <assimp/scene.h>

namespace ntf::shogle {

struct TextureData {
  TextureData(const char* path, GLenum tex_dim, aiTextureType type);
  ~TextureData();

  unsigned char* data;
  aiTextureType type;
  GLenum tex_dim;
  int width;
  int height;
  int nr_channels;
};

}
