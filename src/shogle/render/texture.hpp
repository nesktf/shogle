#pragma once

#include <shogle/render/render.hpp>

#include <shogle/core/log.hpp>

#include <array>

#define SHOGLE_CUBEMAP_FACES 6

namespace ntf {

enum class tex_format {
  mono = 0,
  rgb,
  rgba,
};

enum class tex_filter {
  nearest = 0,
  linear,
  nearest_mp_nearest,
  nearest_mp_linear,
  linear_mp_linear,
  linear_mp_nearest,
};

enum class tex_wrap {
  repeat = 0,
  mirrored_repeat,
  clamp_edge,
  clamp_border,
};

namespace impl {

constexpr GLint enumtogl(tex_format format) {
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

constexpr GLint enumtogl(tex_filter filter) {
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

constexpr GLint enumtogl(tex_wrap wrap) {
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

} // namespace impl

class framebuffer;

class texture2d {
public:
  texture2d() = default;
  inline texture2d(GLuint id, size_t w, size_t h);
  inline texture2d(uint8_t* data, size_t w, size_t h, tex_format format);

public:
  inline texture2d& set_filter(tex_filter filter);
  inline texture2d& set_wrap(tex_wrap wrap);

public:
  GLuint& id() { return _id; } // Not const
  ivec2 dim() const { return _dim; }
  bool valid() const { return _id != 0; }

public:
  inline ~texture2d();
  inline texture2d(texture2d&&) noexcept;
  texture2d(const texture2d&) = delete;
  inline texture2d& operator=(texture2d&&) noexcept;
  texture2d& operator=(const texture2d&) = delete;

private:
  inline void unload_texture();

private:
  GLuint _id{};
  ivec2 _dim{};

private:
  friend void render_bind_sampler(const texture2d& tex, size_t sampler);
  friend void render_bind_sampler(const framebuffer& fb, size_t sampler);
};

inline texture2d load_texture(uint8_t* data, size_t w, size_t h, tex_format format, 
                              tex_filter filter, tex_wrap wrap) {
  texture2d tex{data, w, h, format};
  tex.set_wrap(wrap);
  tex.set_filter(filter);
  return tex;
}


using cmappixels = std::array<uint8_t*, SHOGLE_CUBEMAP_FACES>;
class cubemap {
public:
  cubemap() = default;
  inline cubemap(GLuint id, size_t dim);
  inline cubemap(cmappixels data, size_t dim, tex_format format);

public:
  inline cubemap& set_filter(tex_filter filter);
  inline cubemap& set_wrap(tex_wrap wrap);

public:
  GLuint& id() { return _id; } // Not const
  ivec2 dim() const { return _dim; }
  bool valid() const { return _id != 0; }

public:
  inline ~cubemap();
  inline cubemap(cubemap&&) noexcept;
  cubemap(const cubemap&) = delete;
  inline cubemap& operator=(cubemap&&) noexcept;
  cubemap& operator=(const cubemap&) = delete;

private:
  inline void unload_cubemap();

private:
  GLuint _id{};
  ivec2 _dim{};

private:
  friend void render_bind_sampler(const cubemap& cubemap, size_t sampler);
};

inline cubemap load_cubemap(cmappixels data, size_t dim, tex_format format, tex_filter filter, tex_wrap wrap) {
  cubemap cmap{std::move(data), dim, format};
  cmap.set_filter(filter);
  cmap.set_wrap(wrap);
  return cmap;
}


inline void render_bind_sampler(const texture2d& tex, size_t sampler) {
  constexpr GLint gltype2d = GL_TEXTURE_2D;
  glActiveTexture(GL_TEXTURE0+sampler);
  glBindTexture(gltype2d, tex._id);
}

inline void render_bind_sampler(const cubemap& cubemap, size_t sampler) {
  constexpr GLint gltypecm = GL_TEXTURE_CUBE_MAP;
  glActiveTexture(GL_TEXTURE0+sampler);
  glBindTexture(gltypecm, cubemap._id);
}


texture2d::texture2d(GLuint id, size_t w, size_t h) : _id(id), _dim(w, h) {}

texture2d::texture2d(uint8_t* data, size_t w, size_t h, tex_format format) : _dim(w, h) {
  constexpr GLint gltype2d = GL_TEXTURE_2D;
  const auto glformat = impl::enumtogl(format);

  glGenTextures(1, &_id);
  glBindTexture(gltype2d, _id);

  glTexImage2D(gltype2d, 0, glformat, w, h, 0, glformat, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(gltype2d);

  glBindTexture(gltype2d, 0);

  log::verbose("[shogle::texture2d] Texture loaded (id: {})", _id);
}

texture2d& texture2d::set_filter(tex_filter filter) {
  constexpr GLint gltype2d = GL_TEXTURE_2D;
  const auto glfilter = impl::enumtogl(filter);

  glBindTexture(gltype2d, _id);
  glTexParameteri(gltype2d, GL_TEXTURE_MIN_FILTER, glfilter);
  glTexParameteri(gltype2d, GL_TEXTURE_MAG_FILTER, glfilter);
  glBindTexture(gltype2d, 0);

  return *this;
}

texture2d& texture2d::set_wrap(tex_wrap wrap) {
  constexpr GLint gltype2d = GL_TEXTURE_2D;
  const auto glwrap = impl::enumtogl(wrap);

  glBindTexture(gltype2d, _id);
  glTexParameteri(gltype2d, GL_TEXTURE_WRAP_T, glwrap);
  glTexParameteri(gltype2d, GL_TEXTURE_WRAP_S, glwrap);
  glBindTexture(gltype2d, 0);

  return *this;
}

void texture2d::unload_texture() {
  log::verbose("[shogle::texture2d] Texture unloaded (id: {})", _id);
  glDeleteTextures(1, &_id);
}

texture2d::~texture2d() {
  if (_id) {
    unload_texture();
  }
}

texture2d::texture2d(texture2d&& t) noexcept : _id(std::move(t._id)), _dim(std::move(t._dim)) {
  t._id = 0;
}

texture2d& texture2d::operator=(texture2d&& t) noexcept {
  if (_id) {
    unload_texture();
  }

  _id = std::move(t._id);
  _dim = std::move(t._dim);

  t._id = 0;

  return *this;
}

cubemap::cubemap(GLuint id, size_t dim) : _id(id), _dim(dim) {}

cubemap::cubemap(std::array<uint8_t*, SHOGLE_CUBEMAP_FACES> data, size_t dim, tex_format format) : _dim(dim, dim) {
  constexpr GLint gltypecm = GL_TEXTURE_CUBE_MAP;
  const auto glformat = impl::enumtogl(format);

  glGenTextures(1, &_id);
  glBindTexture(gltypecm, _id);

  auto cmap_face = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
  for (auto& curr : data) {
    glTexImage2D(cmap_face++, 0, glformat, dim, dim, 0, glformat, GL_UNSIGNED_BYTE, curr);
  }
  glGenerateMipmap(gltypecm);

  glBindTexture(gltypecm, 0);

  log::verbose("[shogle::cubemap] Cubemap loaded (id: {})", _id);
}

cubemap& cubemap::set_filter(tex_filter filter) {
  constexpr GLint gltypecm = GL_TEXTURE_CUBE_MAP;
  const auto glfilter = impl::enumtogl(filter);

  glBindTexture(gltypecm, _id);
  glTexParameteri(gltypecm, GL_TEXTURE_MIN_FILTER, glfilter);
  glTexParameteri(gltypecm, GL_TEXTURE_MAG_FILTER, glfilter);
  glBindTexture(gltypecm, 0);

  return *this;
}

cubemap& cubemap::set_wrap(tex_wrap wrap) {
  constexpr GLint gltypecm = GL_TEXTURE_CUBE_MAP;
  const auto glwrap = impl::enumtogl(wrap);

  glBindTexture(gltypecm, _id);
  glTexParameteri(gltypecm, GL_TEXTURE_WRAP_T, glwrap);
  glTexParameteri(gltypecm, GL_TEXTURE_WRAP_S, glwrap);
  glTexParameteri(gltypecm, GL_TEXTURE_WRAP_R, glwrap);
  glBindTexture(gltypecm, 0);

  return *this;
}

void cubemap::unload_cubemap() {
  log::verbose("[shogle::cubemap] Cubemap unloaded (id: {})", _id);
  glDeleteTextures(1, &_id);
}

cubemap::~cubemap() {
  if (_id) {
    unload_cubemap();
  }
}

cubemap::cubemap(cubemap&& c) noexcept : _id(std::move(c._id)), _dim(std::move(c._id)) {
  c._id = 0;
}

cubemap& cubemap::operator=(cubemap&& c) noexcept {
  if (_id) {
    unload_cubemap();
  }

  _id = std::move(c._id);
  _dim = std::move(c._dim);

  c._id = 0;

  return *this;
}

} // namespace ntf
