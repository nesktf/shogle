#pragma once

#include <shogle/core/log.hpp>

#include <shogle/render/render.hpp>

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


// need to forward declare frens
class framebuffer;
void render_bind_sampler(const framebuffer& fb, size_t sampler);


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


template<size_t faces>
struct tex_helper {
  using data_type = std::array<uint8_t*, faces>;
  using dim_type = size_t;
  static constexpr GLint gltype = GL_TEXTURE_CUBE_MAP; // yes please give me a cubemap with INT_MAX faces
};

template<>
struct tex_helper<1u> {
  using data_type = uint8_t*;
  using dim_type = ivec2;
  static constexpr GLint gltype = GL_TEXTURE_2D;
};


template<size_t faces>
class texture {
public:
  using data_type = typename impl::tex_helper<faces>::data_type;
  using dim_type = typename impl::tex_helper<faces>::dim_type;

public:
  texture() = default;
  texture(GLuint id, dim_type dim) : _id(id), _dim(dim) {}
  texture(data_type data, dim_type dim, tex_format format);

public:
  texture& set_filter(tex_filter filter);
  texture& set_wrap(tex_wrap wrap);

public:
  GLuint& id() { return _id; } // Not const
  dim_type dim() const { return _dim; }
  bool valid() const { return _id != 0; }

private:
  void unload();

private:
  static constexpr GLint gltype = impl::tex_helper<faces>::gltype;

private:
  template<size_t _faces>
  friend void ntf::render_bind_sampler(const texture<_faces>& tex, size_t sampler);
  friend void ntf::render_bind_sampler(const framebuffer& fb, size_t sampler); // only used in texture2d 

private:
  GLuint _id{};
  dim_type _dim{};

public:
  ~texture();
  texture(texture&&) noexcept;
  texture(const texture&) = delete;
  texture& operator=(texture&&) noexcept;
  texture& operator=(const texture&) = delete;
};


template<size_t faces>
texture<faces>::texture(data_type data, dim_type dim, tex_format format) : _dim(dim) {
  const auto glformat = impl::enumtogl(format);

  glGenTextures(1, &_id);
  glBindTexture(gltype, _id);

  auto cmap_face = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
  for (auto& face : data) {
    glTexImage2D(cmap_face++, 0, glformat, dim, dim, 0, glformat, GL_UNSIGNED_BYTE, face);
  }
  glGenerateMipmap(gltype);

  glBindTexture(gltype, 0);

  log::verbose("[ntf::texture] Cubemap loaded (id: {})", _id);
}

template<>
inline texture<1u>::texture(data_type data, dim_type dim, tex_format format) : _dim(dim) {
  const auto glformat = impl::enumtogl(format);
  glGenTextures(1, &_id);
  glBindTexture(gltype, _id);

  glTexImage2D(gltype, 0, glformat, dim.x, dim.y, 0, glformat, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(gltype);

  glBindTexture(gltype, 0);

  log::verbose("[ntf::texture] Texture2D loaded (id: {})", _id);
}

template<size_t faces>
void texture<faces>::unload() {
  if (_id) {
    log::verbose("[ntf::texture] Unloaded (id: {})", _id);
    glDeleteTextures(1, &_id);
  }
}

template<size_t faces>
auto texture<faces>::set_filter(tex_filter filter) -> texture& {
  const auto glfilter = impl::enumtogl(filter);

  glBindTexture(gltype, _id);
  glTexParameteri(gltype, GL_TEXTURE_MIN_FILTER, glfilter);
  glTexParameteri(gltype, GL_TEXTURE_MAG_FILTER, glfilter);
  glBindTexture(gltype, 0);

  return *this;
}

template<size_t faces>
auto texture<faces>::set_wrap(tex_wrap wrap) -> texture& {
  const auto glwrap = impl::enumtogl(wrap);

  glBindTexture(gltype, _id);
  glTexParameteri(gltype, GL_TEXTURE_WRAP_T, glwrap);
  glTexParameteri(gltype, GL_TEXTURE_WRAP_S, glwrap);
  if constexpr (faces > 1u) {
    glTexParameteri(gltype, GL_TEXTURE_WRAP_R, glwrap);
  }
  glBindTexture(gltype, 0);

  return *this;
}

template<size_t faces>
texture<faces>::~texture() { unload(); }

template<size_t faces>
texture<faces>::texture(texture&& t) noexcept : _id(std::move(t._id)), _dim(std::move(t._dim)) { t._id = 0; }

template<size_t faces>
auto texture<faces>::operator=(texture&& t) noexcept -> texture& {
  unload();

  _id = std::move(t._id);
  _dim = std::move(t._dim);

  t._id = 0;

  return *this;
}


template<size_t faces>
texture<faces> load_texture(typename texture<faces>::data_type data, typename texture<faces>::dim_type dim, 
                            tex_format format, tex_filter filter, tex_wrap wrap) {
  texture<faces> tex{data, dim, format};
  tex.set_wrap(wrap);
  tex.set_filter(filter);
  return tex;
}

} // namespace impl


using texture2d = impl::texture<1u>;
using cubemap = impl::texture<SHOGLE_CUBEMAP_FACES>;

template<size_t faces>
void render_bind_sampler(const impl::texture<faces>& tex, size_t sampler) {
  glActiveTexture(GL_TEXTURE0+sampler);
  glBindTexture(impl::texture<faces>::gltype, tex._id);
}

} // namespace ntf
