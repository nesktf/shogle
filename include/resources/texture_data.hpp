#pragma once

#include "glad/glad.h"
#include <assimp/scene.h>

namespace ntf::shogle {

class TextureData {
public:
  enum class Type {
    ModelTex,
    SpriteTex,
    FBOTex
  };

public:
  TextureData(const char* path, GLenum tex_dim, aiTextureType ai_type, Type tex_type);
  ~TextureData();

public:
  unsigned char* data;
  aiTextureType ai_type;
  Type tex_type;
  GLenum tex_dim;
  int width;
  int height;
  int nr_channels;
};

}
