#include "resource/texture.hpp"

#include "log.hpp"

#include "stb/stb_image.h"

namespace ntf::shogle::res {

// Texture::data_t
TextureData::TextureData(const char* path, GLenum tex_dim, aiTextureType ai_type, Type tex_type) :
  path(path),
  tex_dim(tex_dim),
  ai_type(ai_type),
  tex_type(tex_type) {
  data = stbi_load(path, &width, &height, &nr_channels, 0);
  if (!data) {
    log::fatal("[TextureData] File not found: {}", path);
  }
}

TextureData::~TextureData() {
  stbi_image_free(data);
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

  this->ai_type  = data->ai_type;
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

  log::debug("[Texture] Created texture (id: {})", this->tex);
}

Texture::~Texture() {
  GLuint id = this->tex;
  glDeleteTextures(1, &this->tex);
  log::debug("[Texture] Deleted texture (id: {})", id);
}

void Texture::bind_material(Shader& shader, size_t tex_num, size_t tex_ind) const {
  std::string uniform_name{this->name+std::to_string(tex_num)};
  glBindTexture(this->tex_dim, this->tex);
  glActiveTexture(GL_TEXTURE0 + tex_ind);
  shader.unif_int(uniform_name.c_str(), tex_ind);
  shader.unif_float("material.col_shiny", 1.0f); // TODO: don't use fixed value
}

} // namespace ntf::shogle::res

