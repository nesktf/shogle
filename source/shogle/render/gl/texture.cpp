#include <shogle/render/gl/texture.hpp>

#include <shogle/core/log.hpp>
#include <shogle/core/error.hpp>

namespace ntf::shogle::gl {

static GLenum to_gl(enum texture::type type) {
  switch (type) {
    case texture::type::tex2d:
      return GL_TEXTURE_2D;
    case texture::type::cubemap:
      return GL_TEXTURE_CUBE_MAP;
  }
  return 0; // shut up gcc
}

static GLenum to_gl(texture::format format) {
  switch (format) {
    case texture::format::mono:
      return GL_RED;
    case texture::format::rgb:
      return GL_RGB;
    case texture::format::rgba:
      return GL_RGBA;
  }
  return 0;
}

static GLint to_gl(texture::filter filter) {
  switch (filter) {
    case texture::filter::linear:
      return GL_LINEAR;
    case texture::filter::nearest:
      return GL_NEAREST;
  }
  return 0;
}

static void setup_param(GLenum type) {
  if (type == GL_TEXTURE_2D) {
    glTexParameteri(type, GL_TEXTURE_WRAP_T, DEFAULT_WRAP_2D);
    glTexParameteri(type, GL_TEXTURE_WRAP_S, DEFAULT_WRAP_2D);
  } else if (type == GL_TEXTURE_CUBE_MAP) {
    glTexParameteri(type, GL_TEXTURE_WRAP_T, DEFAULT_WRAP_CUBEMAP);
    glTexParameteri(type, GL_TEXTURE_WRAP_S, DEFAULT_WRAP_CUBEMAP);
    glTexParameteri(type, GL_TEXTURE_WRAP_R, DEFAULT_WRAP_CUBEMAP);
  }
  glTexParameteri(type, GL_TEXTURE_MIN_FILTER, DEFAULT_FILTER);
  glTexParameteri(type, GL_TEXTURE_MAG_FILTER, DEFAULT_FILTER);
}

gl::texture::texture(vec2sz sz, format format, unsigned char* pixels) :
  _format(to_gl(format)), _type(to_gl(type::tex2d)), _size(sz) {
  assert(pixels && "Invalid pixel data");

  glGenTextures(1, &_id);
  glBindTexture(_type, _id);

  glTexImage2D(_type, 0, _format, sz.w, sz.h, 0, _format, GL_UNSIGNED_BYTE, pixels);
  glGenerateMipmap(_type);

  setup_param(_type);
  glBindTexture(_type, 0);
  log::verbose("[gl::texture] Texture created (id: {}, type: tex2d, empty: false)", _id);
}

gl::texture::texture(vec2sz sz, format format, cubemap_pixels pixels) :
  _format(to_gl(format)), _type(to_gl(type::cubemap)), _size(sz) {
  glGenTextures(1, &_id);
  glBindTexture(_type, _id);

  auto face = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
  for (auto* curr : pixels) {
    assert(curr && "Invalid pixel data");
    glTexImage2D(face++, 0, _format, sz.w, sz.h, 0, _format, GL_UNSIGNED_BYTE, curr);
    // glGenerateMipmap(_type); // ?
  }

  setup_param(_type);
  glBindTexture(_type, 0);
  log::verbose("[gl::texture] Texture created (id: {}, type: cubemap, empty: false)", _id);
}

gl::texture::texture(vec2sz sz, enum type type, format format) :
  _format(to_gl(format)), _type(to_gl(type)), _size(sz) {
  glGenTextures(1, &_id);
  glBindTexture(_type, _id);

  size_t image_count = type == type::cubemap ? CUBEMAP_FACES : 1;
  for (size_t i = 0; i < image_count; ++i) {
    glTexImage2D(_type, 0, _format, sz.w, sz.h, 0, _format, GL_UNSIGNED_BYTE, NULL);
  }

  setup_param(_type);
  glBindTexture(_type, 0);
  log::verbose("[gl::texture] Texture created (id: {}, type: {}, empty: true)", _id, type==type::cubemap ? "cubemap" : "tex2d");
}

texture::texture(texture&& t) noexcept :
  _id(t._id), _format(t._format), _type(t._type),
  _filter(t._filter), _size(t._size) { t._id = 0; }

texture& texture::operator=(texture&& t) noexcept {
  auto tex_id = _id;
  auto tex_type = _type;
  glDeleteTextures(1, &_id);

  _id = t._id;
  _format = t._format;
  _type = t._type;
  _filter = t._filter;
  _size = t._size;

  t._id = 0;

  log::verbose("[gl::texture] Texture overwritten (id: {}, type: {})", tex_id, tex_type == GL_TEXTURE_2D ? "tex2d" : "cubemap");
  return *this;
}

texture::~texture() {
  if (!_id) return;
  auto tex_id = _id;
  auto tex_type = _type;
  glDeleteTextures(1, &_id);
  log::verbose("[gl::texture] Texture destroyed (id: {}, type: {})", tex_id, tex_type == GL_TEXTURE_2D ? "tex2d" : "cubemap");
}

texture& texture::set_filter(filter filter) {
  _filter = to_gl(filter);
  glBindTexture(_type, _id);
  glTexParameteri(_type, GL_TEXTURE_MIN_FILTER, _filter);
  glTexParameteri(_type, GL_TEXTURE_MAG_FILTER, _filter);
  glBindTexture(_type, 0);

  return *this;
}

void texture::bind(size_t sampler) const {
  glActiveTexture(GL_TEXTURE0+sampler);
  glBindTexture(_type, _id);
}

} // namespace ntf::shogle::gl
