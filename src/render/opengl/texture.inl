#define SHOGLE_RENDER_OPENGL_TEXTURE_INL
#include "./texture.hpp"
#undef SHOGLE_RENDER_OPENGL_TEXTURE_INL

namespace ntf {

template<std::size_t faces>
void gl_texture<faces>::load(data_type data, dim_type dim, tex_format format) {
  NTF_ASSERT(_id == 0, "gl_texture already initialized");

  const auto glformat = enumtogl(format);
  glGenTextures(1, &_id);
  glBindTexture(gltype, _id);
  if constexpr (faces > 1u) {
    auto cmap_face = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
    for (auto& face : data) {
      glTexImage2D(cmap_face++, 0, glformat, dim, dim, 0, glformat, GL_UNSIGNED_BYTE, face);
    }
  } else {
    glTexImage2D(gltype, 0, glformat, dim.x, dim.y, 0, glformat, GL_UNSIGNED_BYTE, data);
  }

  glGenerateMipmap(gltype);
  glBindTexture(gltype, 0);

  SHOGLE_LOG(verbose, "[ntf::gl_texture] Texture loaded (id: {}, faces: {})", _id, face_count);
}

template<std::size_t faces>
void gl_texture<faces>::unload() {
  if (_id) {
    SHOGLE_LOG(verbose, "[ntf::gl_texture] Texture unloaded (id: {})", _id);
    glDeleteTextures(1, &_id);
    _id = 0;
  }
}

template<std::size_t faces>
auto gl_texture<faces>::set_filter(tex_filter filter) -> gl_texture& {
  NTF_ASSERT(valid(), "Invalid gl_texture");
  const auto glfilter = enumtogl(filter);

  glBindTexture(gltype, _id);
  glTexParameteri(gltype, GL_TEXTURE_MIN_FILTER, glfilter);
  glTexParameteri(gltype, GL_TEXTURE_MAG_FILTER, glfilter);
  glBindTexture(gltype, 0);

  return *this;
}

template<std::size_t faces>
auto gl_texture<faces>::set_wrap(tex_wrap wrap) -> gl_texture& {
  NTF_ASSERT(valid(), "Invalid gl_texture");
  const auto glwrap = enumtogl(wrap);

  glBindTexture(gltype, _id);
  glTexParameteri(gltype, GL_TEXTURE_WRAP_T, glwrap);
  glTexParameteri(gltype, GL_TEXTURE_WRAP_S, glwrap);
  if constexpr (faces > 1u) {
    glTexParameteri(gltype, GL_TEXTURE_WRAP_R, glwrap);
  }
  glBindTexture(gltype, 0);

  return *this;
}

template<std::size_t faces>
void gl_texture<faces>::bind_sampler(std::size_t sampler) const {
  NTF_ASSERT(valid(), "Invalid gl_texture");
  glActiveTexture(GL_TEXTURE0+sampler);
  glBindTexture(gltype, _id);
}

template<std::size_t faces>
gl_texture<faces>::~gl_texture() noexcept { unload(); }

template<std::size_t faces>
gl_texture<faces>::gl_texture(gl_texture&& t) noexcept :
  _id(std::move(t._id)), _dim(std::move(t._dim)) { t._id = 0; }

template<std::size_t faces>
auto gl_texture<faces>::operator=(gl_texture&& t) noexcept -> gl_texture& {
  unload();

  _id = std::move(t._id);
  _dim = std::move(t._dim);

  t._id = 0;

  return *this;
}

} // namespace ntf
