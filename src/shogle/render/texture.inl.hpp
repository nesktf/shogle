#define SHOGLE_TEXTURE_INL_HPP
#include <shogle/render/texture.hpp>
#undef SHOGLE_TEXTURE_INL_HPP

#include <shogle/core/log.hpp>

namespace ntf::shogle {

constexpr GLint __enumtogl(tex_format format) {
  switch (format) {
    case tex_format::rgb:
      return GL_RGB;
    case tex_format::mono:
      return GL_RED;
    case tex_format::rgba:
      return GL_RGBA;
  }
  return 0; // shutup gcc
}

constexpr GLint __enumtogl(tex_filter filter) {
  switch (filter) {
    case tex_filter::linear:
      return GL_LINEAR;
    case tex_filter::nearest:
      return GL_NEAREST;
    case tex_filter::nearest_mp_nearest:
      return GL_NEAREST_MIPMAP_NEAREST;
    case tex_filter::nearest_mp_linear:
      return GL_NEAREST_MIPMAP_LINEAR;
    case tex_filter::linear_mp_linear:
      return GL_LINEAR_MIPMAP_LINEAR;
    case tex_filter::linear_mp_nearest:
      return GL_LINEAR_MIPMAP_NEAREST;
  }
  return 0; // shutup gcc
}

constexpr GLint __enumtogl(tex_wrap wrap) {
  switch (wrap) {
    case tex_wrap::repeat:
      return GL_REPEAT;
    case tex_wrap::mirrored_repeat:
      return GL_MIRRORED_REPEAT;
    case tex_wrap::clamp_edge:
      return GL_CLAMP_TO_EDGE;
    case tex_wrap::clamp_border:
      return GL_CLAMP_TO_BORDER;
  }
  return 0; // shutup gcc
}

constexpr const char* __texlog(GLint type) {
  if (type == GL_TEXTURE_2D) {
    return "tex2d";
  } else if(type == GL_TEXTURE_CUBE_MAP) {
    return "cubemap";
  }
  return "NONE";
}

template<typename T>
__texture<T>::__texture(vec2sz dim, tex_format format, tex_filter filter, tex_wrap wrap, T data) :
  _dim(dim),
  _format(__enumtogl(format)), _filter(__enumtogl(filter)), _wrap(__enumtogl(wrap)) {
  static_assert(_type != GL_NONE, "Invalid texture data type");

  glGenTextures(1, &_id);
  glBindTexture(_type, _id);

  glTexParameteri(_type, GL_TEXTURE_WRAP_T, _wrap);
  glTexParameteri(_type, GL_TEXTURE_WRAP_S, _wrap);
  if constexpr(_type == GL_TEXTURE_CUBE_MAP) {
    glTexParameteri(_type, GL_TEXTURE_WRAP_R, _wrap);
  }

  glTexParameteri(_type, GL_TEXTURE_MIN_FILTER, _filter);
  glTexParameteri(_type, GL_TEXTURE_MAG_FILTER, _filter);

  if constexpr(_type == GL_TEXTURE_2D) {
    assert(data && "Invalid tex2d pixel data");
    glTexImage2D(_type, 0, _format, _dim.w, _dim.h, 0, _format, GL_UNSIGNED_BYTE, data);
  } else if (_type == GL_TEXTURE_CUBE_MAP) {
    auto face = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
    for (auto* curr : data) {
      assert(curr && "Invalid cubemap pixel data");
      glTexImage2D(face++, 0, _format, _dim.w, _dim.h, 0, _format, GL_UNSIGNED_BYTE, curr);
    }
  }
  glGenerateMipmap(_type);

  glBindTexture(_type, 0);
  log::verbose("[shogle::texture] Texture created (id: {}, type: {})", _id, __texlog(_type));
}

template<typename T>
__texture<T>::__texture(vec2sz dim, tex_format format, tex_filter filter, tex_wrap wrap) :
  _dim(dim),
  _format(__enumtogl(format)), _filter(__enumtogl(filter)), _wrap(__enumtogl(wrap)) {
  static_assert(_type != GL_NONE, "Invalid texture data type");

  glGenTextures(1, &_id);
  glBindTexture(_type, _id);

  glTexParameteri(_type, GL_TEXTURE_WRAP_T, _wrap);
  glTexParameteri(_type, GL_TEXTURE_WRAP_S, _wrap);
  if constexpr(_type == GL_TEXTURE_CUBE_MAP) {
    glTexParameteri(_type, GL_TEXTURE_WRAP_R, _wrap);
  }

  glTexParameteri(_type, GL_TEXTURE_MIN_FILTER, _filter);
  glTexParameteri(_type, GL_TEXTURE_MAG_FILTER, _filter);

  size_t texcount = _type == GL_TEXTURE_CUBE_MAP ? CUBEMAP_FACES : 1;
  for (size_t i = 0; i < texcount; ++i) {
    glTexImage2D(_type, 0, _format, _dim.w, _dim.h, 0, _format, GL_UNSIGNED_BYTE, NULL);
  }
  glGenerateMipmap(_type);

  log::verbose("[shogle::texture] Empty texture created (id: {}, type: {})", _id, __texlog(_type));
  glBindTexture(_type, 0);
}

template<typename T>
__texture<T>::~__texture() {
  auto id = _id;
  if (!_id) return;
  glDeleteTextures(1, &_id);
  log::verbose("[shogle::texture] Texture deleted (id: {}, type: {})", id, __texlog(_type));
}

template<typename T>
__texture<T>::__texture(__texture&& tex) noexcept :
  _dim(std::move(tex._dim)),
  _format(std::move(tex._format)), _filter(std::move(tex._filter)), _wrap(std::move(tex._wrap)),
  _id(std::move(tex._id)) {
  tex._id = 0;
}

template<typename T>
inline auto __texture<T>::operator=(__texture&& tex) noexcept -> __texture& {
  auto id = _id;
  glDeleteTextures(_type, &_id);

  _dim = std::move(tex._dim);
  _format = std::move(tex._format);
  _filter = std::move(tex._filter);
  _wrap = std::move(tex._wrap);
  _id = std::move(tex._id);

  tex._id = 0;

  log::verbose("[shogle::texture] Texture overwritten (id: {}, type: {})", id, __texlog(_type));
  return *this;
}

template<typename T>
inline auto __texture<T>::set_filter(tex_filter filter) -> __texture& {
  _filter = __enumtogl(filter);
  glBindTexture(_type, _id);
  glTexParameteri(_type, GL_TEXTURE_MIN_FILTER, _filter);
  glTexParameteri(_type, GL_TEXTURE_MAG_FILTER, _filter);
  glBindTexture(_type, 0);
  return *this;
}

template<typename T>
inline auto __texture<T>::set_wrap(tex_wrap wrap) -> __texture& {
  _wrap = __enumtogl(wrap);
  glBindTexture(_type, _id);
  glTexParameteri(_type, GL_TEXTURE_WRAP_T, _wrap);
  glTexParameteri(_type, GL_TEXTURE_WRAP_S, _wrap);
  if constexpr (_type == GL_TEXTURE_CUBE_MAP) {
    glTexParameteri(_type, GL_TEXTURE_WRAP_R, _wrap);
  }
}

template<typename T>
inline auto __texture<T>::bind_sampler(size_t sampler) -> void {
  glActiveTexture(GL_TEXTURE0+sampler);
  glBindTexture(_type, _id);
}

} // namespace ntf::shogle
