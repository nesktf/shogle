#include <shogle/render/res/texture.hpp>

#include <shogle/core/log.hpp>

#include <stb/stb_image.h>

#define DEFAULT_FILTER GL_NEAREST
#define DEFAULT_WRAP GL_REPEAT

namespace ntf::render {

// Texture
Texture::Texture(const Texture::data_t* data) :
  _w(static_cast<size_t>(data->width)), 
  _h(static_cast<size_t>(data->height)),
  _dim(data->dim) {

  GLenum _format;
  switch (data->channels) {
    case 1:
      _format = GL_RED;
      break;
    case 4:
      _format = GL_RGBA;
      break;
    default:
      _format = GL_RGB;
  }

  glGenTextures(1, &_tex);
  glBindTexture(_dim, _tex);
  glTexImage2D(_dim, 0, _format, _w, _h, 0, _format, GL_UNSIGNED_BYTE, data->tex_data);
  glGenerateMipmap(_dim);
  glTexParameteri(_dim, GL_TEXTURE_MIN_FILTER, DEFAULT_FILTER);
  glTexParameteri(_dim, GL_TEXTURE_MAG_FILTER, DEFAULT_FILTER);
  glTexParameteri(_dim, GL_TEXTURE_WRAP_T, DEFAULT_WRAP);
  glTexParameteri(_dim, GL_TEXTURE_WRAP_S, DEFAULT_WRAP);
  glBindTexture(_dim, 0);

  Log::verbose("[Texture] Texture created (tex-id: {})", _tex);
}

Texture::Texture(size_t w, size_t h, GLenum format, GLenum dim) :
  _w(w),
  _h(h),
  _dim(dim) {

  glGenTextures(1, &_tex);
  glBindTexture(_dim, _tex);
  glTexImage2D(_dim, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(_dim, GL_TEXTURE_MIN_FILTER, DEFAULT_FILTER);
  glTexParameteri(_dim, GL_TEXTURE_MAG_FILTER, DEFAULT_FILTER);
  glTexParameteri(_dim, GL_TEXTURE_WRAP_T, DEFAULT_WRAP);
  glTexParameteri(_dim, GL_TEXTURE_WRAP_S, DEFAULT_WRAP);
  glBindTexture(_dim, 0);
}

Texture::Texture(Texture&& tx) noexcept :
  _w(std::move(tx._w)),
  _h(std::move(tx._h)),
  _tex(std::move(tx._tex)),
  _dim(std::move(tx._dim)) {

  tx._tex = 0;
}

Texture& Texture::operator=(Texture&& tx) noexcept {
  _w = std::move(tx._w);
  _h = std::move(tx._h);
  _tex = std::move(tx._tex);
  _dim = std::move(tx._dim);

  tx._tex = 0;

  return *this;
}

Texture::~Texture() {
  if (!_tex) return;
  GLuint id = _tex;
  glDeleteTextures(1, &_tex);
  Log::verbose("[Texture] Deleted texture (tex-id: {})", id);
}

void Texture::set_filter(GLint filter) {
  glBindTexture(_dim, _tex);
  glTexParameteri(_dim, GL_TEXTURE_MIN_FILTER, filter);
  glTexParameteri(_dim, GL_TEXTURE_MAG_FILTER, filter);
  glBindTexture(_dim, 0);
}

// if (this->ai_type == aiTextureType_DIFFUSE) {
//   this->name = "material.texture_difusse";
// } else {
//   this->name = "material.texture_specular";
// }
// void Texture::bind_material(const Shader& shader, size_t tex_num, size_t tex_ind) const {
//   std::string uniform_name{this->name+std::to_string(tex_num)};
//   glBindTexture(_dim, this->tex);
//   glActiveTexture(GL_TEXTURE0 + tex_ind);
//   shader.unif_int(uniform_name.c_str(), tex_ind);
//   shader.unif_float("material.col_shiny", 1.0f); // TODO: don't use fixed value
// }
//
// if (data->tex_type == TextureData::Type::FBOTex) {
//   glBindFramebuffer(GL_FRAMEBUFFER, data->fbo);
//   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->tex, 0);
// }

} // namespace ntf::render
