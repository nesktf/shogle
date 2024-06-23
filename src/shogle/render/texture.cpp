#include <shogle/render/texture.hpp>

#include <shogle/core/log.hpp>

namespace ntf::shogle {

static constexpr GLint __enumtogl(tex_format format) {
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

static constexpr GLint __enumtogl(tex_filter filter) {
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

static constexpr GLint __enumtogl(tex_wrap wrap) {
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

texture2d::texture2d(uint8_t* data, size_t w, size_t h, tex_format format, tex_filter filter, tex_wrap wrap) :
  _dim(w, h),
  _format(__enumtogl(format)), _filter(__enumtogl(filter)), _wrap(__enumtogl(wrap)) {
  const constexpr GLint type = GL_TEXTURE_2D;

  glGenTextures(1, &_id);
  glBindTexture(type, _id);

  glTexParameteri(type, GL_TEXTURE_WRAP_T, _wrap);
  glTexParameteri(type, GL_TEXTURE_WRAP_S, _wrap);

  glTexParameteri(type, GL_TEXTURE_MIN_FILTER, _filter);
  glTexParameteri(type, GL_TEXTURE_MAG_FILTER, _filter);

  glTexImage2D(type, 0, _format, _dim.w, _dim.h, 0, _format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(type);

  glBindTexture(type, 0);

  log::verbose("[shogle::texture2d] Texture loaded (id: {})", _id);
}

texture2d& texture2d::set_filter(tex_filter filter) {
  const constexpr GLint type = GL_TEXTURE_2D;

  _filter = __enumtogl(filter);

  glBindTexture(type, _id);
  glTexParameteri(type, GL_TEXTURE_MIN_FILTER, _filter);
  glTexParameteri(type, GL_TEXTURE_MAG_FILTER, _filter);
  glBindTexture(type, 0);

  return *this;
}

texture2d& texture2d::set_wrap(tex_wrap wrap) {
  const constexpr GLint type = GL_TEXTURE_2D;

  _wrap = __enumtogl(wrap);

  glBindTexture(type, _id);
  glTexParameteri(type, GL_TEXTURE_WRAP_T, _wrap);
  glTexParameteri(type, GL_TEXTURE_WRAP_S, _wrap);

  return *this;
}

texture2d::~texture2d() {
  if (!_id) return;

  log::verbose("[shogle::texture2d] Texture unloaded (id: {})", _id);
  glDeleteTextures(1, &_id);
}

texture2d::texture2d(texture2d&& t) noexcept :
  _dim(std::move(t._dim)),
  _format(std::move(t._format)), _filter(std::move(t._filter)), _wrap(std::move(t._wrap)),
  _id(std::move(t._id)) {
  t._id = 0;
}

texture2d& texture2d::operator=(texture2d&& t) noexcept {
  if (_id) {
    glDeleteTextures(1, &_id);
    log::verbose("[shogle::texture2d] Texture unloaded (id: {})", _id);
  }

  _dim = std::move(t._dim);
  _format = std::move(t._format);
  _filter = std::move(t._filter);
  _wrap = std::move(t._wrap);
  _id = std::move(t._id);

  t._id = 0;

  return *this;
}

cubemap::cubemap(std::array<uint8_t*, CUBEMAP_FACES> data, size_t dim, tex_format format, tex_filter filter, tex_wrap wrap) :
  _dim(dim, dim),
  _format(__enumtogl(format)), _filter(__enumtogl(filter)), _wrap(__enumtogl(wrap)) {
  const constexpr GLint type = GL_TEXTURE_CUBE_MAP;

  glGenTextures(1, &_id);
  glBindTexture(type, _id);

  glTexParameteri(type, GL_TEXTURE_WRAP_T, _wrap);
  glTexParameteri(type, GL_TEXTURE_WRAP_S, _wrap);
  glTexParameteri(type, GL_TEXTURE_WRAP_R, _wrap);

  glTexParameteri(type, GL_TEXTURE_MIN_FILTER, _filter);
  glTexParameteri(type, GL_TEXTURE_MAG_FILTER, _filter);

  for (auto& curr : data) {
    glTexImage2D(type, 0, _format, _dim.w, _dim.h, 0, _format, GL_UNSIGNED_BYTE, curr);
  }
  glGenerateMipmap(type);

  glBindTexture(type, 0);

  log::verbose("[shogle::cubemap] Cubemap loaded (id: {})", _id);
}

cubemap& cubemap::set_filter(tex_filter filter) {
  const constexpr GLint type = GL_TEXTURE_CUBE_MAP;

  _filter = __enumtogl(filter);

  glBindTexture(type, _id);
  glTexParameteri(type, GL_TEXTURE_MIN_FILTER, _filter);
  glTexParameteri(type, GL_TEXTURE_MAG_FILTER, _filter);
  glBindTexture(type, 0);

  return *this;
}

cubemap& cubemap::set_wrap(tex_wrap wrap) {
  const constexpr GLint type = GL_TEXTURE_CUBE_MAP;

  _wrap = __enumtogl(wrap);

  glBindTexture(type, _id);
  glTexParameteri(type, GL_TEXTURE_WRAP_T, _wrap);
  glTexParameteri(type, GL_TEXTURE_WRAP_S, _wrap);
  glTexParameteri(type, GL_TEXTURE_WRAP_R, _wrap);
  glBindTexture(type, 0);

  return *this;
}

cubemap::~cubemap() {
  if (!_id) return;

  log::verbose("[shogle::cubemap] Cubemap unloaded (id: {})", _id);
  glDeleteTextures(1, &_id);
}

cubemap::cubemap(cubemap&& c) noexcept :
  _dim(std::move(c._dim)),
  _format(std::move(c._format)), _filter(std::move(c._filter)), _wrap(std::move(c._wrap)),
  _id(std::move(c._id)) {
  c._id = 0;
}

cubemap& cubemap::operator=(cubemap&& c) noexcept {
  if (_id) {
    log::verbose("[shogle::cubemap] Cubemap unloaded (id: {})", _id);
    glDeleteTextures(1, &_id);
  }

  _dim = std::move(c._dim);
  _format = std::move(c._format);
  _filter = std::move(c._filter);
  _wrap = std::move(c._wrap);
  _id = std::move(c._wrap);

  c._id = 0;

  return *this;
}

void render_bind_sampler(const texture2d& texture, size_t sampler) {
  glActiveTexture(GL_TEXTURE0+sampler);
  glBindTexture(GL_TEXTURE_2D, texture._id);
}

void render_bind_sampler(const cubemap& cubemap, size_t sampler) {
  glActiveTexture(GL_TEXTURE0+sampler);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap._id);
}

} // namespace ntf::shogle
