#include "resource/texture.hpp"

#include "log.hpp"

#include "stb/stb_image.h"

namespace ntf::shogle::res {

// Texture::data_t
TextureData::TextureData(std::string _path) :
  path(_path),
  tex_dim(GL_TEXTURE_2D),
  ai_type(aiTextureType_DIFFUSE),
  tex_type(Type::SpriteTex) {
  data = stbi_load(path.c_str(), &width, &height, &nr_channels, 0);
  if (!data) {
    Log::fatal("[TextureData] File not found: {}", path);
  }
  Log::verbose("[TextureData] Texture data extracted (path: {})", path);
}
TextureData::TextureData(std::string _path, GLenum _tex_dim, aiTextureType _ai_type, Type _tex_type) :
  path(_path),
  tex_dim(_tex_dim),
  ai_type(_ai_type),
  tex_type(_tex_type) {
  data = stbi_load(path.c_str(), &width, &height, &nr_channels, 0);
  if (!data) {
    Log::fatal("[TextureData] File not found: {}", path);
  }
  Log::verbose("[TextureData] Texture data extracted (path: {})", path);
}

TextureData::TextureData(TextureData&& tx) noexcept :
  path(std::move(tx.path)),
  tex_dim(std::move(tx.tex_dim)),
  ai_type(std::move(tx.ai_type)),
  tex_type(std::move(tx.tex_type)),
  width(std::move(tx.width)),
  height(std::move(tx.height)),
  nr_channels(std::move(tx.nr_channels)),
  data(std::move(tx.data)){

  tx.data = nullptr;
}

TextureData& TextureData::operator=(TextureData&& tx) noexcept {
  this->path = std::move(tx.path);
  this->tex_dim = std::move(tx.tex_dim);
  this->ai_type = std::move(tx.ai_type);
  this->tex_type = std::move(tx.tex_type);
  this->width = std::move(tx.width);
  this->height = std::move(tx.height);
  this->nr_channels = std::move(tx.nr_channels);
  this->data = std::move(tx.data);

  tx.data = nullptr;

  return *this;
}

TextureData::~TextureData() {
  if (data) {
    stbi_image_free(data);
  }
}

// Texture
Texture::Texture(const Texture::data_t* data) {
  // TODO: Handle Framebuffer Textures
  GLenum format;
  switch (data->nr_channels) {
    case 1:
      format = GL_RED;
      break;
    case 4:
      format = GL_RGBA;
      break;
    default:
      format = GL_RGB;
  }
  GLint filter;
  // if (data->tex_type == TextureData::Type::SpriteTex || data->tex_type == TextureData::Type::FBOTex) {
  if (data->tex_type == TextureData::Type::SpriteTex) {
    filter = GL_NEAREST;
  } else {
    filter = GL_LINEAR;
  }

  this->ai_type   = data->ai_type;
  this->tex_dim   = data->tex_dim;
  this->width     = data->width;
  this->height    = data->height;

  if (this->ai_type == aiTextureType_DIFFUSE) {
    this->name = "material.texture_difusse";
  } else {
    this->name = "material.texture_specular";
  }

  glGenTextures(1, &this->tex);
  glBindTexture(this->tex_dim, this->tex);
  glTexImage2D(this->tex_dim, 0, format, this->width, this->height, 0, format, GL_UNSIGNED_BYTE, data->data);
  glGenerateMipmap(this->tex_dim);
  glTexParameteri(this->tex_dim, GL_TEXTURE_MIN_FILTER, filter);
  glTexParameteri(this->tex_dim, GL_TEXTURE_MAG_FILTER, filter);
  glTexParameteri(this->tex_dim, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(this->tex_dim, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glBindTexture(this->tex_dim, 0);

  // if (data->tex_type == TextureData::Type::FBOTex) {
  //   glBindFramebuffer(GL_FRAMEBUFFER, data->fbo);
  //   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->tex, 0);
  // }

  Log::verbose("[Texture] Texture created (tex-id: {})", this->tex);
}

Texture::Texture(Texture&& tx) noexcept :
  name(std::move(tx.name)),
  width(std::move(tx.width)),
  height(std::move(tx.height)),
  tex(std::move(tx.tex)),
  tex_dim(std::move(tx.tex_dim)),
  ai_type(std::move(tx.ai_type)) {

  tx.tex = 0;
}

Texture& Texture::operator=(Texture&& tx) noexcept {
  this->name = std::move(tx.name);
  this->width = std::move(tx.width);
  this->height = std::move(tx.height);
  this->tex = std::move(tx.tex);
  this->tex_dim = std::move(tx.tex_dim);
  this->ai_type = std::move(tx.ai_type);

  tx.tex = 0;

  return *this;
}

Texture::~Texture() {
  if (this->tex == 0) return;
  GLuint id = this->tex;
  glDeleteTextures(1, &this->tex);
  Log::verbose("[Texture] Deleted texture (tex-id: {})", id);
}

void Texture::bind_material(const Shader& shader, size_t tex_num, size_t tex_ind) const {
  std::string uniform_name{this->name+std::to_string(tex_num)};
  glBindTexture(this->tex_dim, this->tex);
  glActiveTexture(GL_TEXTURE0 + tex_ind);
  shader.unif_int(uniform_name.c_str(), tex_ind);
  shader.unif_float("material.col_shiny", 1.0f); // TODO: don't use fixed value
}

} // namespace ntf::shogle::res

