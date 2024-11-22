#define SHOGLE_RENDER_OPENGL_TEXTURE_INL
#include "./texture.hpp"
#undef SHOGLE_RENDER_OPENGL_TEXTURE_INL

namespace ntf {

template<std::size_t faces>
gl_texture<faces>::gl_texture(data_type data, dim_type dim, tex_format format,
                              gl_tex_params params) {
  _load(data, dim, format, params);

}

template<std::size_t faces>
auto gl_texture<faces>::load(data_type data, dim_type dim, tex_format format,
                             gl_tex_params params) & -> gl_texture& {
  _load(data, dim, format, params);
  return *this;
}

template<std::size_t faces>
auto gl_texture<faces>::load(data_type data, dim_type dim, tex_format format,
                             gl_tex_params params) && -> gl_texture&& {
  _load(data, dim, format, params);
  return std::move(*this);
}

template<std::size_t faces>
auto gl_texture<faces>::filter(tex_filter filter) & -> gl_texture& {
  _set_filter(filter, true);
  return *this;
}

template<std::size_t faces>
auto gl_texture<faces>::wrap(tex_wrap wrap) & -> gl_texture& {
  _set_wrap(wrap, true);
  return *this;
}

template<std::size_t faces>
void gl_texture<faces>::unload() {
  if (!_id) {
    return;
  }

  SHOGLE_LOG(verbose, "[ntf::gl_texture] Texture destroyed (id: {})", _id);
  glDeleteTextures(1, &_id);

  _reset();
}


template<std::size_t faces>
void gl_texture<faces>::_load(data_type data, dim_type dim, tex_format format,
                              gl_tex_params params) {
  const auto glformat = enumtogl(format);
  GLuint id{};
  glGenTextures(1, &id);
  glBindTexture(gltype, id);
  if constexpr (faces > 1u) {
    auto cmap_face = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
    for (auto& face : data) {
      glTexImage2D(cmap_face++, 0, glformat, dim, dim, 0, glformat, GL_UNSIGNED_BYTE, face);
    }
  } else {
    glTexImage2D(gltype, 0, glformat, dim.x, dim.y, 0, glformat, GL_UNSIGNED_BYTE, data);
  }

  if (params.gen_mipmaps) {
    glGenerateMipmap(gltype);
  }

  _set_filter(params.filter, false);
  _set_wrap(params.wrap, false);
  glBindTexture(gltype, 0);

  if (_id) {
    SHOGLE_LOG(verbose, "[ntf::gl_texture] Texture overwritten ({} -> {})", _id, id);
    glDeleteTextures(1, &_id);
  } else {
    SHOGLE_LOG(verbose, "[ntf::gl_texture] Texture created (id: {}, faces: {})", _id, face_count);
  }

  _id = id;
  _dim = dim;
}

template<std::size_t faces>
void gl_texture<faces>::_set_filter(tex_filter filter, bool bind) {
  NTF_ASSERT(valid(), "Invalid gl_texture");
  const auto glfilter = enumtogl(filter);

  if (bind) {
    glBindTexture(gltype, _id);
  }

  glTexParameteri(gltype, GL_TEXTURE_MAG_FILTER, glfilter);
  glTexParameteri(gltype, GL_TEXTURE_MIN_FILTER, glfilter);

  if (bind) {
    glBindTexture(gltype, 0);
  }

  return *this;
}

template<std::size_t faces>
void gl_texture<faces>::_set_wrap(tex_wrap wrap, bool bind) {
  NTF_ASSERT(valid(), "Invalid gl_texture");
  const auto glwrap = enumtogl(wrap);

  if (bind) {
    glBindTexture(gltype, _id);
  }

  glTexParameteri(gltype, GL_TEXTURE_WRAP_T, glwrap);
  glTexParameteri(gltype, GL_TEXTURE_WRAP_S, glwrap);
  if constexpr (faces > 1u) {
    glTexParameteri(gltype, GL_TEXTURE_WRAP_R, glwrap);
  }

  if (bind) {
    glBindTexture(gltype, 0);
  }

  return *this;
}

template<std::size_t faces>
void gl_texture<faces>::_reset() {
  _id = 0;
}


template<std::size_t faces>
gl_texture<faces>::~gl_texture() noexcept { unload(); }

template<std::size_t faces>
gl_texture<faces>::gl_texture(gl_texture&& t) noexcept :
  _id(std::move(t._id)), _dim(std::move(t._dim)) { t._reset(); }

template<std::size_t faces>
auto gl_texture<faces>::operator=(gl_texture&& t) noexcept -> gl_texture& {
  unload();

  _id = std::move(t._id);
  _dim = std::move(t._dim);

  t._reset();

  return *this;
}

} // namespace ntf
