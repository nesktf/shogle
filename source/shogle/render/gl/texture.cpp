#include <shogle/render/gl/texture.hpp>

#include <shogle/core/log.hpp>
#include <shogle/core/error.hpp>

#define DEFAULT_FILTER GL_NEAREST
#define DEFAULT_WRAP_2D GL_REPEAT
#define DEFAULT_WRAP_CUBEMAP GL_CLAMP_TO_EDGE
#define CUBEMAP_FACES 6

namespace ntf::shogle::gl {

gl::texture::texture(vec2sz sz, enum type type, format format, char** pixels) :
  _size(sz) {
  switch (type) {
    case type::tex2d: {
      _type = GL_TEXTURE_2D;
      break;
    }
    case type::cubemap: {
      _type = GL_TEXTURE_CUBE_MAP;
      break;
    }
  }
  switch (format) {
    case format::mono: {
      _format = GL_RED;
      break;
    }
    case format::rgb: {
      _format = GL_RGB;
      break;
    }
    case format::rgba: {
      _format = GL_RGBA;
      break;
    }
  }

  glGenTextures(1, &_id);
  glBindTexture(_type, _id);
  if (type == type::cubemap) {
    for (size_t i = 0; i < CUBEMAP_FACES; ++i) {
      // right, left, top, bottom, back, front
      auto face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
      glTexImage2D(face, 0, _format, sz.w, sz.h, 0, _format, GL_UNSIGNED_BYTE, pixels != NULL ? pixels[i] : NULL);
    }

    glTexParameteri(_type, GL_TEXTURE_WRAP_T, DEFAULT_WRAP_CUBEMAP);
    glTexParameteri(_type, GL_TEXTURE_WRAP_S, DEFAULT_WRAP_CUBEMAP);
    glTexParameteri(_type, GL_TEXTURE_WRAP_R, DEFAULT_WRAP_CUBEMAP);
  } else {
    glTexImage2D(_type, 0, _format, sz.w, sz.h, 0, _format, GL_UNSIGNED_BYTE, pixels != NULL ? *pixels : NULL);
    glGenerateMipmap(_type);
    glTexParameteri(_type, GL_TEXTURE_WRAP_T, DEFAULT_WRAP_2D);
    glTexParameteri(_type, GL_TEXTURE_WRAP_S, DEFAULT_WRAP_2D);
  }
  glTexParameteri(_type, GL_TEXTURE_MIN_FILTER, DEFAULT_FILTER);
  glTexParameteri(_type, GL_TEXTURE_MAG_FILTER, DEFAULT_FILTER);
  glBindTexture(_type, 0);

  Log::verbose("[gl::texture] Texture created (id: {}, type: {}, empty: {})", _id, _type == GL_TEXTURE_2D ? "tex2d" : "cubemap", pixels == NULL);
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

  Log::verbose("[gl::texture] Texture overwritten (id: {}, type: {})", tex_id, tex_type == GL_TEXTURE_2D ? "tex2d" : "cubemap");
  return *this;
}

texture::~texture() {
  if (!_id) return;
  auto tex_id = _id;
  auto tex_type = _type;
  glDeleteTextures(1, &_id);
  Log::verbose("[gl::texture] Texture destroyed (id: {}, type: {})", tex_id, tex_type == GL_TEXTURE_2D ? "tex2d" : "cubemap");
}

texture& texture::set_filter(filter filter) {
  switch (filter) {
    case texture::filter::nearest: {
      _filter = GL_NEAREST;
      break;
    };
    case texture::filter::linear: {
      _filter = GL_LINEAR;
      break;
    }
  };
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
