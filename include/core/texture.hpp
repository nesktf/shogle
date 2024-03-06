#pragma once

#include "glad/glad.h"
#include "core/shader.hpp"
#include "resources/texture_data.hpp"
#include <assimp/texture.h>
#include <memory>

namespace ntf::shogle {

class Texture {
public:
  using data_t = TextureData;

public:
  Texture(std::unique_ptr<TextureData> data);
  // Raw pointer constructor should only be used for mesh texture initialization
  Texture(const TextureData* data);
  ~Texture();

  Texture(Texture&&) = default;
  Texture& operator=(Texture&&) = default;

  Texture(const Texture&) = delete;
  Texture& operator=(Texture&) = delete;

public:
  void bind_material(Shader& shader, size_t tex_num, size_t tex_ind) const;

public:
  float aspect(void) const { return (float)width/(float)height;}
  GLuint id(void) const { return tex; }
  GLenum dim(void) const { return tex_dim; }
  aiTextureType type(void) const { return ai_type; }

private:
  std::string name;
  int width, height;
  GLuint tex;
  GLenum tex_dim;
  aiTextureType ai_type;
};

} 
