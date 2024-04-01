#pragma once

#include "resource/shader.hpp"

#include <assimp/texture.h>
#include <assimp/scene.h>

#include <memory>

namespace ntf::shogle::res {

// Texture::data_t
class TextureData {
public:
  enum class Type {
    ModelTex,
    SpriteTex,
    FBOTex
  };

public: // Resource data can be copied but i don't think is a good idea
  TextureData(std::string path);
  TextureData(std::string path, GLenum tex_dim, aiTextureType ai_type, Type tex_type);

  ~TextureData();

  TextureData(TextureData&&) noexcept;
  TextureData& operator=(TextureData&&) noexcept;

  TextureData(const TextureData&) = delete;
  TextureData& operator=(const TextureData&) = delete;

public:
  std::string path;
  GLenum tex_dim;
  aiTextureType ai_type;
  Type tex_type;
  int width;
  int height;
  int nr_channels;
  unsigned char* data;
};

// Texture
class Texture {
public:
  using data_t = TextureData;

public: // Resources can't be copied
  Texture(const Texture::data_t* data);
  ~Texture();

  Texture(Texture&&) noexcept;
  Texture& operator=(Texture&&) noexcept;

  Texture(const Texture&) = delete;
  Texture& operator=(Texture&) = delete;

public:
  void bind_material(const Shader& shader, size_t tex_num, size_t tex_ind) const;

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

} // namespace ntf::shogle::res

