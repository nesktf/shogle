#define SHOGLE_RENDER_TEXTURE_INL
#include <shogle/render/gl/texture.hpp>
#undef SHOGLE_RENDER_TEXTURE_INL

namespace ntf {

template<size_t faces>
gl::texture<faces>::texture(data_type data, dim_type dim, tex_format format) : _dim(dim) {
  const auto glformat = renderer::enumtogl(format);

  glGenTextures(1, &_id);
  glBindTexture(gltype, _id);

  auto cmap_face = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
  for (auto& face : data) {
    glTexImage2D(cmap_face++, 0, glformat, dim, dim, 0, glformat, GL_UNSIGNED_BYTE, face);
  }
  glGenerateMipmap(gltype);

  glBindTexture(gltype, 0);

  SHOGLE_INTERNAL_LOG_FMT(verbose, "[SHOGLE][ntf::gl::texture] Loaded (id: {}, faces: {})", _id, face_count);
}

template<>
inline gl::texture<1u>::texture(data_type data, dim_type dim, tex_format format) : _dim(dim) {
  const auto glformat = renderer::enumtogl(format);
  glGenTextures(1, &_id);
  glBindTexture(gltype, _id);

  glTexImage2D(gltype, 0, glformat, dim.x, dim.y, 0, glformat, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(gltype);

  glBindTexture(gltype, 0);

  SHOGLE_INTERNAL_LOG_FMT(verbose, 
    "[SHOGLE][ntf::gl::texture] Loaded (id: {}, faces: {}, empty: {})", _id, face_count, data == nullptr);
}

template<size_t faces>
gl::texture<faces>::~texture() noexcept { 
#ifdef SHOGLE_GL_RAII_UNLOAD
  unload();
#endif
}

template<size_t faces>
gl::texture<faces>::texture(texture&& t) noexcept :
  _id(std::move(t._id)), _dim(std::move(t._dim)) { t._id = 0; }

template<size_t faces>
auto gl::texture<faces>::operator=(texture&& t) noexcept -> texture& {
  unload();

  _id = std::move(t._id);
  _dim = std::move(t._dim);

  t._id = 0;

  return *this;
}

template<size_t faces>
void gl::texture<faces>::unload() {
  if (_id) {
    SHOGLE_INTERNAL_LOG_FMT(verbose, "[SHOGLE][ntf::gl::texture] Unloaded (id: {})", _id);
    glDeleteTextures(1, &_id);
    _id = 0;
  }
}

template<size_t faces>
auto gl::texture<faces>::set_filter(tex_filter filter) -> texture& {
  const auto glfilter = renderer::enumtogl(filter);

  glBindTexture(gltype, _id);
  glTexParameteri(gltype, GL_TEXTURE_MIN_FILTER, glfilter);
  glTexParameteri(gltype, GL_TEXTURE_MAG_FILTER, glfilter);
  glBindTexture(gltype, 0);

  return *this;
}

template<size_t faces>
auto gl::texture<faces>::set_wrap(tex_wrap wrap) -> texture& {
  const auto glwrap = renderer::enumtogl(wrap);

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
void gl::texture<faces>::bind_sampler(size_t sampler) const {
  glActiveTexture(GL_TEXTURE0+sampler);
  glBindTexture(gltype, _id);
}

} // namespace ntf
